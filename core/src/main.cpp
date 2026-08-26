#include <algorithm>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "lattice/concurrent_index.hpp"
#include "lattice/hnsw.hpp"
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

// Queries hammering the index while a writer inserts into it.
//
// What this checks: no crashes, no garbage results, no data races.
// It does NOT check that queries see every insert - a query running
// concurrently with ingestion is allowed to miss recent writes. What
// it must never do is return a corrupted or impossible result.
void cmd_stress(size_t total_inserts, size_t reader_threads,
                size_t seconds) {
    lattice::ConcurrentIndex index;
    auto data = make_dataset(total_inserts, 42);

    // Seed it so readers have something to search from the start.
    const size_t seed_count = std::min<size_t>(500, data.size());
    for (size_t i = 0; i < seed_count; ++i) {
        index.insert(data[i]);
    }
    std::cout << "seeded with " << seed_count << " vectors\n";

    std::atomic<bool> stop{false};
    std::atomic<uint64_t> bad_results{0};
    std::atomic<uint64_t> empty_results{0};

    // Readers.
    std::vector<std::thread> readers;
    for (size_t t = 0; t < reader_threads; ++t) {
        readers.emplace_back([&, t]() {
            std::mt19937 rng(static_cast<uint32_t>(9000 + t));

            while (!stop.load(std::memory_order_relaxed)) {
                auto query = random_vector(rng);
                auto hits = index.search(query, 10, 50);

                if (hits.empty()) {
                    empty_results.fetch_add(1, std::memory_order_relaxed);
                    continue;
                }

                // Sanity-check every result. A distance that's negative,
                // NaN, or absurdly large means the reader saw a node in
                // a half-written state.
                float prev = -1.0f;
                for (const auto& h : hits) {
                    const bool bad_distance =
                        !(h.distance >= 0.0f) ||        // catches NaN too
                        h.distance > 1e6f ||
                        h.distance < prev;              // must be ascending
                    if (bad_distance || h.id == 0) {
                        bad_results.fetch_add(1, std::memory_order_relaxed);
                        break;
                    }
                    prev = h.distance;
                }
            }
        });
    }

    // Writer.
    std::thread writer([&]() {
        for (size_t i = seed_count; i < data.size(); ++i) {
            if (stop.load(std::memory_order_relaxed)) break;
            index.insert(data[i]);
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    stop.store(true, std::memory_order_relaxed);

    writer.join();
    for (auto& r : readers) r.join();

    std::cout << "\n--- stress result ---\n";
    std::cout << "reader threads:  " << reader_threads << "\n";
    std::cout << "inserts done:    " << index.insert_count() << "\n";
    std::cout << "searches done:   " << index.search_count() << "\n";
    std::cout << "final size:      " << index.size() << "\n";
    std::cout << "empty results:   " << empty_results.load() << "\n";
    std::cout << "BAD results:     " << bad_results.load() << "\n";

    if (bad_results.load() == 0) {
        std::cout << "\nno corrupted results observed\n";
    } else {
        std::cout << "\n*** CORRUPTION DETECTED ***\n";
    }
}

// Does concurrent insertion produce the same graph as single-threaded?
//
// Inserts the same data both ways, then checks the same queries give
// the same answers. Since there's only one writer thread either way,
// insertion order is identical and the graphs should match exactly.
void cmd_equiv(size_t count, size_t queries) {
    auto data = make_dataset(count, 42);

    // Single-threaded reference.
    lattice::HnswIndex plain;
    for (const auto& v : data) plain.insert(v);

    // Through the concurrent wrapper, with readers running alongside.
    lattice::ConcurrentIndex guarded;
    std::atomic<bool> stop{false};

    std::vector<std::thread> readers;
    for (size_t t = 0; t < 4; ++t) {
        readers.emplace_back([&, t]() {
            std::mt19937 rng(static_cast<uint32_t>(500 + t));
            while (!stop.load(std::memory_order_relaxed)) {
                auto q = random_vector(rng);
                (void)guarded.search(q, 10, 50);
            }
        });
    }

    for (const auto& v : data) guarded.insert(v);

    stop.store(true, std::memory_order_relaxed);
    for (auto& r : readers) r.join();

    std::cout << "single-threaded size: " << plain.size() << "\n";
    std::cout << "concurrent size:      " << guarded.size() << "\n";
    std::cout << "searches during build: " << guarded.search_count() << "\n\n";

    // Same queries against both.
    std::mt19937 qrng(1337);
    size_t identical = 0;
    size_t differing = 0;

    for (size_t i = 0; i < queries; ++i) {
        auto query = random_vector(qrng);

        auto a = plain.search(query, 10, 50);
        auto b = guarded.search(query, 10, 50);

        bool same = (a.size() == b.size());
        if (same) {
            for (size_t j = 0; j < a.size(); ++j) {
                if (a[j].id != b[j].id) {
                    same = false;
                    break;
                }
            }
        }

        if (same) {
            identical++;
        } else {
            differing++;
            if (differing <= 3) {
                std::cout << "  query " << i << " differs:\n    plain: ";
                for (const auto& r : a) std::cout << r.id << " ";
                std::cout << "\n    guarded: ";
                for (const auto& r : b) std::cout << r.id << " ";
                std::cout << "\n";
            }
        }
    }

    std::cout << "identical results: " << identical << " / " << queries
              << "\n";
    if (differing > 0) {
        std::cout << "DIFFERING: " << differing << "\n";
    }
}

// What do the locks actually cost? Same total queries, varying threads.
void cmd_scaling(size_t count, size_t queries_per_thread) {
    auto data = make_dataset(count, 42);

    lattice::ConcurrentIndex index;
    for (const auto& v : data) index.insert(v);

    std::cout << "index built with " << index.size() << " vectors\n\n";
    std::cout << std::left << std::setw(10) << "threads" << std::setw(14)
              << "total queries" << std::setw(12) << "wall ms"
              << "queries/sec\n";
    std::cout << std::string(48, '-') << "\n";

    for (size_t threads : {1u, 2u, 4u, 8u}) {
        std::vector<std::thread> workers;
        const auto t0 = std::chrono::steady_clock::now();

        for (size_t t = 0; t < threads; ++t) {
            workers.emplace_back([&, t]() {
                std::mt19937 rng(static_cast<uint32_t>(100 + t));
                for (size_t q = 0; q < queries_per_thread; ++q) {
                    auto query = random_vector(rng);
                    (void)index.search(query, 10, 50);
                }
            });
        }

        for (auto& w : workers) w.join();
        const auto t1 = std::chrono::steady_clock::now();

        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            t1 - t0).count();
        const size_t total = threads * queries_per_thread;
        const double qps = (ms > 0)
            ? (static_cast<double>(total) * 1000.0 / static_cast<double>(ms))
            : 0.0;

        std::cout << std::left << std::setw(10) << threads << std::setw(14)
                  << total << std::setw(12) << ms << std::fixed
                  << std::setprecision(0) << qps << "\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage:\n"
                  << "  scratch stress <inserts> <readers> <seconds>\n"
                  << "  scratch equiv <count> <queries>\n"
                  << "  scratch scaling <count> <queries_per_thread>\n";
        return 1;
    }

    const std::string cmd = argv[1];

    if (cmd == "stress") {
        cmd_stress(argc >= 3 ? std::stoull(argv[2]) : 20000,
                   argc >= 4 ? std::stoull(argv[3]) : 4,
                   argc >= 5 ? std::stoull(argv[4]) : 5);
    } else if (cmd == "equiv") {
        cmd_equiv(argc >= 3 ? std::stoull(argv[2]) : 5000,
                  argc >= 4 ? std::stoull(argv[3]) : 100);
    } else if (cmd == "scaling") {
        cmd_scaling(argc >= 3 ? std::stoull(argv[2]) : 20000,
                    argc >= 4 ? std::stoull(argv[3]) : 2000);
    } else {
        std::cout << "unknown command: " << cmd << "\n";
        return 1;
    }

    return 0;
}