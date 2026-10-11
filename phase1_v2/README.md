# Phase 1 (v2): all trips in memory, a distance search, no threads

The first version in `../phase1` is kept as our v0 (one pass over the file, 282 s).

## Files

- `include/taxi_trip.h`: one trip, every column as a normal C++ type (`extra` is skipped, it is always empty)
- `include/csv_reader.h`, `src/csv_reader.cpp`: the `CSVReader` class, reads the CSV one trip at a time (columns are in a fixed order)
- `include/trip_data.h`, `src/trip_data.cpp`: loads all trips into a `std::vector<TaxiTrip>`, the search `searchByDistance` (count and add up the trips in a distance range) and `lookupTrips` (read all fields of the trips with the given IDs)
- `src/main.cpp`: loads, runs the distance report (5 distance searches) and the trip lookups (1M random IDs, 5 rounds), prints times and memory, appends one line to `results/phase1_v2_runs.csv`

## Build (gcc 13+ or Homebrew clang 16+, not Apple's Xcode clang)

```bash
cd phase1_v2
CXX=g++-16 cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/taxi_phase1 ~/Downloads/taxi2014/trips_every7th.csv     # one run
./run_10_times.sh   ~/Downloads/taxi2014/trips_every7th.csv     # 10 runs
```

An optional second number limits how many trips are loaded (0 = all). One `TaxiTrip` is 216 bytes with g++,
so all 71.7M trips need about 15 GB, more than an 8 GB laptop has; that is why we use `trips_every7th.csv`
(see the main README, section 3). Limiting the full file with a number does not work well: the file is in
date order, so the first N trips are all from January.
