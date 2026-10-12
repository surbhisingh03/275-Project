#!/usr/bin/env bash
# Step 2 (compact arrays) on the FULL file at 1, 2, 4 and 8 threads, RUNS runs each.
# Usage: ./scripts/full_sweep.sh <full_file.csv> [RUNS]
set -u
PROJECT="$(cd "$(dirname "$0")/.." && pwd)"
FULL_FILE="$1"; RUNS="${2:-3}"
mkdir -p "$PROJECT/phase2/full_data/results"
for T in 1 2 4 8; do
  for run in $(seq 1 "$RUNS"); do
    echo "$(date +%H:%M:%S) full file, $T threads, run $run"
    (cd "$PROJECT/phase2/full_data" && OMP_NUM_THREADS=$T ../build/taxi_compact "$FULL_FILE" > "results/run_t${T}_${run}.txt")
  done
done
echo "$(date +%H:%M:%S) full sweep done"
