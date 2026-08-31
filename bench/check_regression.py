"""Compare a benchmark run against the stored baseline.

Exits non-zero if recall or latency has regressed past the allowed
threshold, which is what makes the CI job fail.

Thresholds are deliberately asymmetric. Recall is deterministic given
the same data and parameters, so it should barely move at all - 5% is
already generous. Latency varies a lot between machines and between
runs on the same machine, since CI runners are shared and noisy
neighbours are real, so 25% is loose on purpose. A tighter latency
gate would fail constantly for reasons that have nothing to do with
the code being tested.
"""

import argparse
import json
import sys

RECALL_TOLERANCE = 0.05   # 5% relative drop allowed
LATENCY_TOLERANCE = 0.25  # 25% relative increase allowed


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--baseline", default="baseline.json")
    ap.add_argument("--current", required=True,
                    help="JSON file produced by run.py --json")
    args = ap.parse_args()

    with open(args.baseline) as f:
        base = json.load(f)
    with open(args.current) as f:
        cur = json.load(f)

    failures = []
    notes = []

    # Recall: lower is worse.
    recall_floor = base["recall"] * (1.0 - RECALL_TOLERANCE)
    if cur["recall"] < recall_floor:
        failures.append(
            f"recall {cur['recall']:.4f} is below the floor "
            f"{recall_floor:.4f} (baseline {base['recall']:.4f}, "
            f"tolerance {RECALL_TOLERANCE:.0%})"
        )
    else:
        notes.append(
            f"recall {cur['recall']:.4f} vs baseline {base['recall']:.4f} - ok"
        )

    # Latency: higher is worse.
    for key, label in (("p50_us", "p50"), ("p99_us", "p99")):
        ceiling = base[key] * (1.0 + LATENCY_TOLERANCE)
        if cur[key] > ceiling:
            failures.append(
                f"{label} {cur[key]:.0f}us exceeds the ceiling "
                f"{ceiling:.0f}us (baseline {base[key]:.0f}us, "
                f"tolerance {LATENCY_TOLERANCE:.0%})"
            )
        else:
            notes.append(
                f"{label} {cur[key]:.0f}us vs baseline {base[key]:.0f}us - ok"
            )

    for n in notes:
        print(f"  {n}")

    if failures:
        print("\nREGRESSION DETECTED:")
        for f in failures:
            print(f"  - {f}")
        print("\nIf this change legitimately shifts performance, update "
              "bench/baseline.json deliberately and say why in the commit "
              "message. Do not update it just to make this pass.")
        sys.exit(1)

    print("\nno regression")
    sys.exit(0)


if __name__ == "__main__":
    main()