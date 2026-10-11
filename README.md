# CmpE 275 Mini 1: NYC Yellow Taxi 2014, Jan–May

We load NYC taxi trips into memory and run two kinds of query on them:

- **Distance report** (a few fields of ALL trips): 5 distance searches, one per bucket (0–1, 1–3, 3–5, 5–10,
  over 10 miles). Each search counts the matching trips and adds up their fares, distances, tips and totals.
- **Trip lookups by ID** (ALL fields of a few trips): 1,000,000 random trip IDs, like a help desk pulling up
  the full record of each trip a passenger asks about. The ID is the trip's row number (the data has no ID
  column); the seed is fixed, so every run looks up the same trips. Done 5 times: round 1 and round 5 are saved.

Phase 1 does this the plain object-oriented way; Phase 2 adds threads (step 1) and then a compact layout (step 2).

| Folder | What it is |
| --- | --- |
| `phase1/` | v0, the first version (one pass over the file). Kept for reference; it expects the portal's CSV export format, not the API download below |
| `phase1_v2/` | Phase 1: all trips in a `std::vector<TaxiTrip>`, the distance report, no threads |
| `phase2/` | Phase 2: `step1_threads/` (Phase 1 design + OpenMP) and `step2_compact/` (one array per field, the `taxistore` library) |
| `scripts/` | Download, run-everything, summary and memory-timeline scripts |
| `results_summary.md` | All measurements side by side (made by `scripts/summarize_results.py`) |

All commands below are run from the `275-Project` folder unless they say otherwise.

## 1. Requirements

- macOS or Linux
- **g++ 13 or newer** (we used `g++-16` from Homebrew) or Homebrew/LLVM clang 16+. Not Apple's Xcode clang: it has no OpenMP.
- CMake 3.20+
- Python 3 (matplotlib only for the charts, see section 6)
- `curl`, and about 20 GB of free disk space

```bash
brew install gcc cmake      # macOS: gives g++-16 and cmake
```

## 2. Download the data

Dataset: [2014 Yellow Taxi Trip Data](https://data.cityofnewyork.us/Transportation/2014-Yellow-Taxi-Trip-Data/gkne-dk5s/about_data)
(NYC Open Data, `gkne-dk5s`), pickups from 1 January to 31 May 2014.

```bash
caffeinate -i ./scripts/download_data.sh ~/Downloads/taxi2014 2>&1 | tee ~/Downloads/taxi2014-download.log
```

- It makes one request per day (151 days), 12 at a time, and took about 100 minutes for us.
- If it ends with "N days missing", run the same command again; finished days are skipped.
- Optional settings at the start of the command: `PAR=8` (fewer requests at once), `APP_TOKEN=...` (a free NYC Open Data app token, less throttling).
- Result: `~/Downloads/taxi2014/yellow_tripdata_2014_jan_may.csv`, **71,667,210 trips, about 15 GB**. The script prints the count; when it matches, delete `~/Downloads/taxi2014/days/`.

(`caffeinate -i` is macOS only; it stops the Mac sleeping. Leave it out on Linux.)

## 3. Make the test file (every 7th trip)

As objects, all 71.7M trips need about 14.4 GB, more than an 8 GB laptop has. Phase 1 and step 1 therefore
run on a smaller file, and every version uses that same file so the numbers can be compared.

```bash
awk 'NR==1 || (NR-2)%7==0' ~/Downloads/taxi2014/yellow_tripdata_2014_jan_may.csv \
    > ~/Downloads/taxi2014/trips_every7th.csv
```

This keeps the header and every 7th trip: **10,238,173 trips, 2.0 GB**, about 2 million from each month.
Don't use `head` instead: the full file is in date order, so its first 10 million trips are all from January,
and the results would describe January only.

## 4. Build each project

Build each one on its own, in its own folder. Both default to an optimised (`-O3`) Release build.

**Phase 1:**

```bash
cd phase1_v2
CXX=g++-16 cmake -S . -B build
cmake --build build          # makes build/taxi_phase1
cd ..
```

**Phase 2** (both steps):

```bash
cd phase2
CXX=g++-16 cmake -S . -B build
cmake --build build          # makes build/taxi_threads (step 1) and build/taxi_compact (step 2)
cd ..
```

If you change compiler, delete the `build` folder first. `CXX` is only read the first time CMake runs.

## 5. Run each program once

Run each program **from its own folder**: it saves its numbers to `results/…csv` inside the folder it is run from.

**Phase 1:**

```bash
cd phase1_v2
./build/taxi_phase1 ~/Downloads/taxi2014/trips_every7th.csv
cd ..
```

A second number limits how many trips are loaded (`0` or nothing = all).

**Phase 2.** `OMP_NUM_THREADS` sets the number of threads:

```bash
cd phase2
FILE=~/Downloads/taxi2014/trips_every7th.csv

OMP_NUM_THREADS=8 ./build/taxi_threads $FILE        # step 1: objects + threads
OMP_NUM_THREADS=8 ./build/taxi_threads $FILE old    # step 1 with Phase 1's parsing (timegm, stod)
OMP_NUM_THREADS=8 ./build/taxi_compact $FILE        # step 2: compact arrays + threads

# step 2 is the only version that can load the whole file on an 8 GB Mac
OMP_NUM_THREADS=8 ./build/taxi_compact ~/Downloads/taxi2014/yellow_tripdata_2014_jan_may.csv
cd ..
```

Each run prints the trips loaded, load time, trip data and peak memory, the distance report and the trip
lookups, and adds one line to `results/phase1_v2_runs.csv` or `results/phase2_runs.csv`.

## 6. Benchmarks

Before measuring: **close Chrome and other heavy apps**, and keep the laptop plugged in.

### Everything at once (about 1.5 hours)

```bash
caffeinate -i ./scripts/run_everything.sh \
    ~/Downloads/taxi2014/trips_every7th.csv \
    ~/Downloads/taxi2014/yellow_tripdata_2014_jan_may.csv \
    2>&1 | tee run_everything.log
```

| Stage | What it runs | Time |
| --- | --- | --- |
| `build` | Builds both projects | 1 min |
| `phase1` | Phase 1, 10 runs | ~15 min |
| `phase2` | Step 1 (Phase 1 parsing: 3 runs; our parsing: 10 runs) and step 2 (10 runs), at 1, 2, 4 and 8 threads | ~65 min |
| `fulldata` | Step 2 on all 71.7M trips, 8 threads, 3 runs | ~3 min |
| `memory` | Memory-over-time recordings of Phase 1, step 1 and step 2 | ~2 min |
| `summary` | Writes `results_summary.md` and the charts | seconds |

- **Run only some stages** by naming them, e.g. `STAGES="phase2 summary" ./scripts/run_everything.sh <test file> <full file>`.
- **Earlier results are never overwritten:** before measuring, the script moves them to `old_results/<date-time>/`.

### One phase at a time

```bash
./phase1_v2/run_10_times.sh ~/Downloads/taxi2014/trips_every7th.csv      # Phase 1, 10 runs
./phase2/run_all.sh        ~/Downloads/taxi2014/trips_every7th.csv      # Phase 2, 1/2/4/8 threads
./phase2/run_all.sh        ~/Downloads/taxi2014/trips_every7th.csv 5 2  # 5 runs each, 2 for "old" parsing
```

## 7. Results, summary and charts

The charts need matplotlib. Install it once in a virtual environment:

```bash
python3 -m venv .venv
source .venv/bin/activate          # run this again in every new terminal
python3 -m pip install matplotlib
```

Then:

```bash
python3 scripts/summarize_results.py          # writes results_summary.md (no matplotlib needed)
(cd phase2 && python3 make_charts.py)         # phase2/results/signal1_threads.png, signal2_data_size.png

# memory over time: record any programs, then plot them on one chart
python3 scripts/memory_timeline.py record phase1 phase1_v2/build/taxi_phase1 ~/Downloads/taxi2014/trips_every7th.csv
OMP_NUM_THREADS=8 python3 scripts/memory_timeline.py record step2 phase2/build/taxi_compact ~/Downloads/taxi2014/trips_every7th.csv
python3 scripts/memory_timeline.py plot       # memory_results/memory_timeline.png
```

### Where results are stored

| File | Contents |
| --- | --- |
| `phase1_v2/results/phase1_v2_runs.csv` | One line per Phase 1 run |
| `phase2/results/phase2_runs.csv` | One line per Phase 2 run (version, threads, times, memory, lookup times) |
| `phase2/full_data/results/phase2_runs.csv` | Step 2 on the full file |
| `*/results/*.txt` | The full printed output of each run; used to check that every version found the same trips |
| `memory_results/` | Memory-over-time recordings (CSV) and `memory_timeline.png` |
| `results_summary.md` | Everything above, side by side |

Only compare runs made on the same data file. `make_charts.py` uses runs with one trip count, and lists any it left out.
