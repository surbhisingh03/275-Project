# Phase 2: threads, then compact data

Two changes to phase1_v2, measured separately. The same queries as phase 1, so results can be compared:
the distance report (5 buckets, a few fields of all trips) and the trip lookups by ID (1M random IDs, all
fields of each trip). The lookups are where we expect objects to beat compact arrays.

| Folder | What it is |
| --- | --- |
| `common/` | Code both steps use: `TaxiTrip` and `SearchResult` (`taxi_trip.h`), and `CSVReader` (`csv_reader.h/.cpp`), which splits the file so each thread reads and parses its own part |
| `step1_threads/` | **Change 1.** Phase 1 design (`std::vector<TaxiTrip>`) + OpenMP threads for loading and searching (`trip_data.h/.cpp`) |
| `step2_compact/` | **Change 2.** One array per field with smaller types (57 bytes per trip instead of 216). Built as its own library, `taxistore` (`trip_store.h/.cpp`), that `main` links to |

## What we found while building it

Loading got **slower** with more threads. The cause was two functions phase 1 uses:

- `timegm` (turns a date into seconds): very slow, and on macOS calls wait for a shared lock, so 4 threads took about 5x LONGER than 1 thread for the same work.
- `std::stod` (turns text into a number): also slows down a bit with threads.

We replaced them with our own date math (days per month + leap years) and `std::from_chars`. Run any program with `old` at the end to use the phase 1 functions again and compare.

## Build (gcc 13+ or Homebrew clang 16+; Apple's Xcode clang has no OpenMP)

```bash
cd phase2
CXX=g++-16 cmake -S . -B build
cmake --build build
```

## Run

```bash
FILE=~/Downloads/taxi2014/yellow_tripdata_2014_jan_may.csv

OMP_NUM_THREADS=4 ./build/taxi_threads $FILE        # change 1, our parsing
OMP_NUM_THREADS=4 ./build/taxi_threads $FILE old    # change 1, phase 1 parsing
OMP_NUM_THREADS=4 ./build/taxi_compact $FILE        # change 2

./run_all.sh $FILE          # everything at 1, 2, 4, 8 threads, 10 runs each (old: 3 runs)
python3 make_charts.py      # charts + averages table (needs: python3 -m pip install matplotlib)
```

Run the programs from the `phase2` folder (the scripts do this for you), and give the data file as a
full path. Every run adds a line to `results/phase2_runs.csv`.

Use the same data file for phase 1 and phase 2, otherwise the numbers cannot be compared. On an 8 GB
laptop the full file does not fit as objects (phase 1 and step 1), so we use `trips_every7th.csv`
(every 7th trip, 10.2M trips; see the main README, section 3). Don't use `head` to make a smaller file:
the full file is in date order, so the first 10 million trips are all from January.

## Charts

`make_charts.py` only uses runs with the **same number of trips**. By default it picks the trip count
with the most phase 2 runs and lists the runs it left out; to choose, pass the trip count:

```bash
python3 make_charts.py 10238173
```

- `results/signal1_threads.png`: load time and distance report time vs threads
- `results/signal2_data_size.png`: trip data and peak memory, and distance report time, at 1 thread
- The table also prints `peak/data`: how much more memory the whole program needed than the trips alone
