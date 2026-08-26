#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

#include "lattice/hnsw.hpp"
#include "lattice/quantizer.hpp"
#include "lattice/search.hpp"
#include "lattice/vector.hpp"

namespace {

constexpr size_t kDim = 8;

std::vector<float> random_vector(std::mt19937& rng) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    std::vector<float> v(kDim);
    for (size_t i = 0; i < kDim; ++i) {
        v[i] = dist(rng);
    }
    return v;
}

std::vector<lattice::Vector> make_dataset(size_t count, uint32_t seed) {
    std::vector<lattice::Vector> out;
    out.reserve(count);

    std::mt19937 rng(seed);
    for (size_t i = 1; i <= count; ++i) {
        lattice::Vector v;
        v.id = static_cast<uint64_t>(i);
        v.data = random_vector(rng);
        out.push_back(std::move(v));
    }
    return out;
}

double recall_at_k(const std::vector<lattice::SearchResult>& truth,
                   const std::vector<lattice::SearchResult>& got) {
    if (truth.empty()) return 1.0;

    std::unordered_set<uint64_t> truth_ids;
    for (const auto& r : truth) truth_ids.insert(r.id);

    size_t hits = 0;
    for (const auto& r : got) {
        if (truth_ids.count(r.id)) hits++;
    }
    return static_cast<double>(hits) / static_cast<double>(truth.size());
}

void cmd_error(size_t count) {
    auto data = make_dataset(count, 42);

    lattice::ScalarQuantizer q;
    q.train(data);

    double total_abs_err = 0.0;
    double max_abs_err = 0.0;
    size_t samples = 0;

    for (const auto& v : data) {
        auto encoded = q.encode(v.data);
        auto decoded = q.decode(encoded);

        for (size_t i = 0; i < v.data.size(); ++i) {
            const double err = std::abs(
                static_cast<double>(v.data[i]) - static_cast<double>(decoded[i]));
            total_abs_err += err;
            max_abs_err = std::max(max_abs_err, err);
            samples++;
        }
    }

    std::cout << "reconstruction error over " << count << " vectors, dim "
              << kDim << "\n";
    std::cout << "  mean abs error: " << std::scientific << std::setprecision(4)
              << (total_abs_err / static_cast<double>(samples)) << "\n";
    std::cout << "  max abs error:  " << max_abs_err << "\n";

    std::cout << "\n  per-dimension scale (bucket width) and max theoretical "
                 "error:\n";
    for (size_t i = 0; i < q.dim(); ++i) {
        std::cout << "    dim " << i << ": scale " << q.scales()[i]
                  << "  max err " << (q.scales()[i] / 2.0f) << "\n";
    }

    std::cout << std::fixed;
    std::cout << "\nmemory per vector:\n";
    std::cout << "  float32: " << (kDim * sizeof(float)) << " bytes\n";
    std::cout << "  uint8:   " << (kDim * sizeof(uint8_t)) << " bytes\n";
    std::cout << "  ratio:   " << (static_cast<double>(sizeof(float)) /
                                   static_cast<double>(sizeof(uint8_t)))
              << "x smaller\n";
}

void cmd_qrecall(size_t count, size_t k, size_t queries) {
    auto data = make_dataset(count, 42);

    lattice::ScalarQuantizer q;
    q.train(data);

    std::vector<std::pair<uint64_t, std::vector<uint8_t>>> encoded;
    encoded.reserve(data.size());
    for (const auto& v : data) {
        encoded.emplace_back(v.id, q.encode(v.data));
    }

    std::mt19937 qrng(1337);

    double total_recall = 0.0;
    long long full_us = 0;
    long long quant_us = 0;

    for (size_t i = 0; i < queries; ++i) {
        auto query = random_vector(qrng);

        auto t0 = std::chrono::steady_clock::now();
        auto truth = lattice::brute_force_search(data, query, k);
        auto t1 = std::chrono::steady_clock::now();

        std::vector<lattice::SearchResult> scored;
        scored.reserve(encoded.size());
        for (const auto& [id, code] : encoded) {
            scored.push_back({id, q.distance_to_query(code, query)});
        }
        std::partial_sort(
            scored.begin(),
            scored.begin() + static_cast<long>(std::min(k, scored.size())),
            scored.end(),
            [](const lattice::SearchResult& a, const lattice::SearchResult& b) {
                return a.distance < b.distance;
            });
        scored.resize(std::min(k, scored.size()));
        auto t2 = std::chrono::steady_clock::now();

        full_us += std::chrono::duration_cast<std::chrono::microseconds>(
                       t1 - t0).count();
        quant_us += std::chrono::duration_cast<std::chrono::microseconds>(
                        t2 - t1).count();

        total_recall += recall_at_k(truth, scored);
    }

    std::cout << "quantized search vs full-precision ground truth\n";
    std::cout << "  vectors:  " << count << "\n";
    std::cout << "  k:        " << k << "\n";
    std::cout << "  queries:  " << queries << "\n\n";
    std::cout << "  recall@" << k << ": " << std::fixed << std::setprecision(4)
              << (total_recall / static_cast<double>(queries)) << "\n";
    std::cout << "  full-precision scan: "
              << (full_us / static_cast<long long>(queries)) << " us/query\n";
    std::cout << "  quantized scan:      "
              << (quant_us / static_cast<long long>(queries)) << " us/query\n";
}

void cmd_sample(size_t count, size_t sample_size, size_t k, size_t queries) {
    auto data = make_dataset(count, 42);

    std::vector<lattice::Vector> sample(
        data.begin(),
        data.begin() + static_cast<long>(std::min(sample_size, data.size())));

    lattice::ScalarQuantizer q;
    q.train(sample);

    std::vector<std::pair<uint64_t, std::vector<uint8_t>>> encoded;
    encoded.reserve(data.size());
    for (const auto& v : data) {
        encoded.emplace_back(v.id, q.encode(v.data));
    }

    std::mt19937 qrng(1337);
    double total_recall = 0.0;

    for (size_t i = 0; i < queries; ++i) {
        auto query = random_vector(qrng);
        auto truth = lattice::brute_force_search(data, query, k);

        std::vector<lattice::SearchResult> scored;
        scored.reserve(encoded.size());
        for (const auto& [id, code] : encoded) {
            scored.push_back({id, q.distance_to_query(code, query)});
        }
        std::partial_sort(
            scored.begin(),
            scored.begin() + static_cast<long>(std::min(k, scored.size())),
            scored.end(),
            [](const lattice::SearchResult& a, const lattice::SearchResult& b) {
                return a.distance < b.distance;
            });
        scored.resize(std::min(k, scored.size()));

        total_recall += recall_at_k(truth, scored);
    }

    std::cout << "trained on " << sample.size() << " of " << count
              << " vectors\n";
    std::cout << "  recall@" << k << ": " << std::fixed << std::setprecision(4)
              << (total_recall / static_cast<double>(queries)) << "\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage:\n"
                  << "  scratch error <count>\n"
                  << "  scratch qrecall <count> <k> <queries>\n"
                  << "  scratch sample <count> <sample_size> <k> <queries>\n";
        return 1;
    }

    const std::string cmd = argv[1];

    if (cmd == "error") {
        cmd_error(argc >= 3 ? std::stoull(argv[2]) : 10000);
    } else if (cmd == "qrecall") {
        cmd_qrecall(argc >= 3 ? std::stoull(argv[2]) : 10000,
                    argc >= 4 ? std::stoull(argv[3]) : 10,
                    argc >= 5 ? std::stoull(argv[4]) : 100);
    } else if (cmd == "sample") {
        cmd_sample(argc >= 3 ? std::stoull(argv[2]) : 10000,
                   argc >= 4 ? std::stoull(argv[3]) : 100,
                   argc >= 5 ? std::stoull(argv[4]) : 10,
                   argc >= 6 ? std::stoull(argv[5]) : 100);
    } else {
        std::cout << "unknown command: " << cmd << "\n";
        return 1;
    }

    return 0;
}