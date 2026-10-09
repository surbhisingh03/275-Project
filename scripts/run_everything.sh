#!/usr/bin/env bash
# Runs every measurement for the report, then writes results_summary.md.
#
# Usage (from the 275-Project folder):
#   caffeinate -i ./scripts/run_everything.sh <test_file.csv> <full_file.csv> 2>&1 | tee run_everything.log
# e.g.
#   caffeinate -i ./scripts/run_everything.sh ~/Downloads/taxi2014/trips_every7th.csv \
#       ~/Downloads/taxi2014/yellow_tripdata_2014_jan_may.csv 2>&1 | tee run_everything.log
#
# Stages (all run by default, in this order). To run only some:  STAGES="phase2 summary" ./scripts/run_everything.sh ...
#   build      build phase1_v2 and phase2 (Release, g++-16)                                   ~1 min
#   phase1     phase 1, 10 runs on the test file                                              ~15 min
#   phase2     step 1 (old + new parsing) and step 2, 1/2/4/8 threads (10 runs, old: 3)        ~65 min
#   cacheoff   step 1 and step 2 with MallocLargeCache=0, 1/2/4/8 threads, 3 runs each       ~7 min
#   fulldata   step 2 on the FULL file, 8 threads, 3 runs (+1 with MallocLargeCache=0)       ~5 min
#   memory     memory-over-time recordings: phase 1, step 1, step 2 (8 threads)              ~2 min
#   summary    results_summary.md + charts
#
# Before starting: close Chrome and other heavy apps, keep the Mac plugged in.
# Old results are moved to old_results/<date-time>/ first, so every run starts clean.

set -u
PROJECT="$(cd "$(dirname "$0")/.." && pwd)"
STAGES="${STAGES:-build phase1 phase2 cacheoff fulldata memory summary}"
CXX_COMPILER="${CXX_COMPILER:-g++-16}"

if [ $# -lt 2 ] || [ ! -f "$1" ] || [ ! -f "$2" ]; then
    echo "Usage: ./scripts/run_everything.sh <test_file.csv> <full_file.csv>"
    exit 1
fi
TEST_FILE="$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
FULL_FILE="$(cd "$(dirname "$2")" && pwd)/$(basename "$2")"

say() { echo "$(date +%H:%M:%S)  $*"; }
has_stage() { [[ " $STAGES " == *" $1 "* ]]; }

cd "$PROJECT"
say "Test file: $TEST_FILE"
say "Full file: $FULL_FILE"
say "Stages:    $STAGES"

# ---- keep old results out of the way ----
if has_stage phase1 || has_stage phase2 || has_stage cacheoff || has_stage fulldata || has_stage memory; then
    OLD="old_results/$(date +%Y%m%d-%H%M%S)"
    for f in phase1_v2/results phase2/results phase2/cache_off/results phase2/full_data/results memory_results; do
        if [ -d "$f" ] && [ -n "$(ls -A "$f" 2>/dev/null)" ]; then
            mkdir -p "$OLD/$f"
            mv "$f"/* "$OLD/$f/"
        fi
    done
    [ -d "$OLD" ] && say "Moved earlier results to $OLD"
fi

# ---- build ----
if has_stage build; then
    say "Building phase1_v2 and phase2 with $CXX_COMPILER"
    (cd phase1_v2 && CXX=$CXX_COMPILER cmake -S . -B build > /dev/null && cmake --build build > /dev/null) || { say "phase1 build FAILED"; exit 1; }
    (cd phase2 && CXX=$CXX_COMPILER cmake -S . -B build > /dev/null && cmake --build build > /dev/null) || { say "phase2 build FAILED"; exit 1; }
fi

# ---- phase 1 ----
if has_stage phase1; then
    say "Phase 1: 10 runs"
    ./phase1_v2/run_10_times.sh "$TEST_FILE" 0 | grep -E "^Run|Load time"
fi

# ---- phase 2 ----
if has_stage phase2; then
    say "Phase 2: step 1 (old + new parsing) and step 2 at 1, 2, 4, 8 threads"
    ./phase2/run_all.sh "$TEST_FILE"
fi

# ---- the allocator cache test (results go to phase2/cache_off/results) ----
if has_stage cacheoff; then
    say "MallocLargeCache=0 runs"
    mkdir -p phase2/cache_off/results
    for threads in 1 2 4 8; do
        for program in taxi_threads taxi_compact; do
            for run in 1 2 3; do
                say "  cache off: $program, $threads threads, run $run"
                (cd phase2/cache_off && MallocLargeCache=0 OMP_NUM_THREADS=$threads ../build/$program "$TEST_FILE" > /dev/null)
            done
        done
    done
fi

# ---- step 2 on the full file (results go to phase2/full_data/results) ----
if has_stage fulldata; then
    say "Full file: step 2, 8 threads"
    mkdir -p phase2/full_data/results phase2/full_data/cache_off/results
    for run in 1 2 3; do
        say "  full file, run $run"
        (cd phase2/full_data && OMP_NUM_THREADS=8 ../build/taxi_compact "$FULL_FILE" > "results/run_$run.txt")
    done
    say "  full file, MallocLargeCache=0"
    (cd phase2/full_data/cache_off && MallocLargeCache=0 OMP_NUM_THREADS=8 ../../build/taxi_compact "$FULL_FILE" > /dev/null)
fi

# ---- memory over time ----
if has_stage memory; then
    say "Memory timelines"
    python3 scripts/memory_timeline.py record phase1_objects phase1_v2/build/taxi_phase1 "$TEST_FILE"
    OMP_NUM_THREADS=8 python3 scripts/memory_timeline.py record step1_objects_8threads phase2/build/taxi_threads "$TEST_FILE"
    OMP_NUM_THREADS=8 python3 scripts/memory_timeline.py record step2_compact_8threads phase2/build/taxi_compact "$TEST_FILE"
fi

# ---- summary and charts ----
if has_stage summary; then
    say "Summary"
    python3 scripts/summarize_results.py
    if python3 -c "import matplotlib" 2>/dev/null; then
        (cd phase2 && python3 make_charts.py > /dev/null && echo "charts: phase2/results/signal1_threads.png, signal2_data_size.png")
        [ -d memory_results ] && python3 scripts/memory_timeline.py plot
    else
        echo "Charts skipped: run  python3 -m pip install matplotlib  and then  STAGES=summary ./scripts/run_everything.sh <test> <full>"
    fi
fi
say "All done."
