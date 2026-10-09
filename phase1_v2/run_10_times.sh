#!/usr/bin/env bash
# Run phase 1 ten times on the same file and rows. Each run adds one line to results/phase1_v2_runs.csv.
# Usage: ./run_10_times.sh <taxi_file.csv> [max_rows]

if [ ! -f "$1" ]; then
    echo "Usage: ./run_10_times.sh <taxi_file.csv> [max_rows]"
    echo "File not found: $1"
    exit 1
fi

# Turn the file name into a full path BEFORE moving into the phase1_v2 folder,
# so a relative path like ../data/taxi.csv still points to the right file.
FILE="$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
MAX_ROWS="${2:-0}"

cd "$(dirname "$0")"
mkdir -p results
for i in 1 2 3 4 5 6 7 8 9 10
do
    echo "Run $i"
    ./build/taxi_phase1 "$FILE" "$MAX_ROWS" > "results/run_$i.txt"
    grep -E "Trips loaded|Load time|Peak memory" "results/run_$i.txt"
done
echo "All runs saved in results/phase1_v2_runs.csv"
