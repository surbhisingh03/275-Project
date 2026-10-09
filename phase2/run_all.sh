#!/usr/bin/env bash
# Run the phase 2 programs with 1, 2, 4 and 8 threads, 10 times each (old parsing: 3 times).
# Every run adds one line to results/phase2_runs.csv.
#   taxi_threads old    = step 1, dates and numbers parsed like phase 1 (timegm, std::stod)
#   taxi_threads        = step 1, our faster parsing
#   taxi_compact        = step 2, compact arrays, our faster parsing
# Usage: ./run_all.sh <taxi_file.csv> [runs] [old_runs]
#   old_runs (default 3): runs for the "old" version, which is very slow with several threads

if [ ! -f "$1" ]; then
    echo "Usage: ./run_all.sh <taxi_file.csv> [runs] [old_runs]"
    echo "File not found: $1"
    exit 1
fi

# Turn the file name into a full path BEFORE moving into the phase2 folder,
# so a relative path like ../data/taxi.csv still points to the right file.
FILE="$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
RUNS="${2:-10}"
OLD_RUNS="${3:-3}"

cd "$(dirname "$0")"
mkdir -p results

for threads in 1 2 4 8
do
    for run in $(seq 1 "$OLD_RUNS")
    do
        echo "taxi_threads old, $threads threads, run $run"
        OMP_NUM_THREADS=$threads ./build/taxi_threads "$FILE" old > "results/taxi_threads_old_t${threads}_run${run}.txt"
    done

    for run in $(seq 1 "$RUNS")
    do
        echo "taxi_threads, $threads threads, run $run"
        OMP_NUM_THREADS=$threads ./build/taxi_threads "$FILE" > "results/taxi_threads_t${threads}_run${run}.txt"
    done

    for run in $(seq 1 "$RUNS")
    do
        echo "taxi_compact, $threads threads, run $run"
        OMP_NUM_THREADS=$threads ./build/taxi_compact "$FILE" > "results/taxi_compact_t${threads}_run${run}.txt"
    done
done
echo "Done. Results in results/phase2_runs.csv"
