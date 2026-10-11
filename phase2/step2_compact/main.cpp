// Phase 2, step 2: compact arrays (the taxistore library) + OpenMP threads.
// Number of threads: OMP_NUM_THREADS=4 ./build/taxi_compact file.csv
// Run with OMP_NUM_THREADS=1 to see the effect of the compact data alone.
#include "taxi_trip.h"
#include "csv_reader.h"
#include "trip_store.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <omp.h>
#include <sys/resource.h>
#include <unistd.h>
#ifdef __APPLE__
#include <libproc.h>
#endif

// seconds since some fixed point, for timing
double now()
{
    auto t = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration<double>(t).count();
}

// biggest amount of memory this program has used so far, in MB
double peakMemoryMB()
{
#ifdef __APPLE__
    // macOS: the "footprint" (what Activity Monitor shows). It also counts memory the system
    // has compressed or moved to disk. ru_maxrss misses those, so on a Mac that is short of
    // RAM it reports too little.
    rusage_info_v4 info;
    proc_pid_rusage(getpid(), RUSAGE_INFO_V4, (rusage_info_t*)&info);
    return info.ri_lifetime_max_phys_footprint / (1024.0 * 1024.0);
#else
    rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss / 1024.0;   // Linux gives kilobytes
#endif
}

// print the count and the averages of one search
void printResult(std::string name, SearchResult r)
{
    std::cout << name << ": " << r.count << " trips" << std::endl;
    if (r.count > 0)
    {
        std::cout << "  Average fare: " << r.fare_sum / r.count << std::endl;
        std::cout << "  Average distance: " << r.distance_sum / r.count << std::endl;
        std::cout << "  Average tip: " << r.tip_sum / r.count << std::endl;
        std::cout << "  Average total: " << r.total_sum / r.count << std::endl;
    }
    if (r.distance_sum > 0)
    {
        std::cout << "  Fare per mile: " << r.fare_sum / r.distance_sum << std::endl;
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: OMP_NUM_THREADS=<n> " << argv[0] << " <taxi_file.csv> [old]" << std::endl;
        std::cout << "  add \"old\" to parse dates and numbers the phase 1 way" << std::endl;
        return 1;
    }
    std::string file_name = argv[1];
    int threads = omp_get_max_threads();   // OMP_NUM_THREADS, or all cores if it is not set
    std::string version = "step2_compact";
    if (argc >= 3 && std::string(argv[2]) == "old")
    {
        use_old_parsing = true;
        version = "step2_compact_old";
    }

    std::cout << version << " (compact arrays + threads), threads: " << threads << std::endl;
    std::cout << "File: " << file_name << std::endl;

    // ---- load ----
    TripStore store;
    double start = now();
    if (!store.load(file_name, threads))
    {
        std::cout << "Could not open " << file_name << std::endl;
        return 1;
    }
    double load_seconds = now() - start;
    double memory_mb = peakMemoryMB();

    // the trip data itself: number of trips x bytes per trip in our arrays (exact, unlike the peak below)
    double data_mb = store.size() * store.bytesPerTrip() / (1024.0 * 1024.0);

    std::cout << "\nTrips loaded: " << store.size() << std::endl;
    std::cout << "Load time: " << load_seconds << " s" << std::endl;
    std::cout << "Bytes per trip in our arrays: " << store.bytesPerTrip()
              << "  (phase 1 TaxiTrip object: " << sizeof(TaxiTrip) << ")" << std::endl;
    std::cout << "Trip data in memory: " << data_mb << " MB" << std::endl;
    std::cout << "Peak memory (whole program): " << memory_mb << " MB" << std::endl;

    // ---- the search: distance report, five buckets ----
    // bucket i is: more than limits[i] miles, up to limits[i+1] miles.
    // The first limit is -1 (not 0) so that trips of exactly 0 miles are counted in the first bucket.
    double limits[] = {-1, 1, 3, 5, 10, 1e12};
    std::string bucket_names[] = {"0-1 mile trips", "1-3 mile trips", "3-5 mile trips",
                                  "5-10 mile trips", "Over-10-mile trips"};
    SearchResult buckets[5];

    start = now();
    for (int i = 0; i < 5; i++)
    {
        buckets[i] = store.searchByDistance(limits[i], limits[i + 1]);
    }
    double distance_seconds = now() - start;

    std::cout << "\nDistance report" << std::endl;
    for (int i = 0; i < 5; i++)
    {
        printResult(bucket_names[i], buckets[i]);
    }
    std::cout << "Distance report time (5 searches): " << distance_seconds << " s" << std::endl;

    // ---- the second query: trip lookups by ID ----
    // Like a help desk answering 1,000,000 passenger requests: each names one trip (its row
    // number, as the data has no trip ID) and needs ALL its fields.
    // Fixed seed, so every version and every run looks up the same trips.
    const long long LOOKUPS = 1000000;
    std::mt19937_64 random_numbers(275);
    std::vector<long long> ids(LOOKUPS);
    for (long long k = 0; k < LOOKUPS; k++)
    {
        ids[k] = random_numbers() % store.size();
    }

    // Done 5 times. The first time can be slow if macOS compressed or swapped part of the
    // trip data while loading; by the 5th time it is back in normal memory ("settled").
    SearchResult looked_up;
    double lookup_first_seconds = 0, lookup_settled_seconds = 0;
    std::cout << "\nTrip lookups (" << LOOKUPS << " random trip IDs, all fields)" << std::endl;
    for (int round = 1; round <= 5; round++)
    {
        start = now();
        looked_up = store.lookupTrips(ids);
        double seconds = now() - start;
        if (round == 1) lookup_first_seconds = seconds;
        lookup_settled_seconds = seconds;
        std::cout << "  Round " << round << ": " << seconds << " s" << std::endl;
    }
    printResult("Trip lookups", looked_up);
    std::cout << "Lookup check: average total " << std::fixed << std::setprecision(2)
              << looked_up.total_sum / looked_up.count << std::defaultfloat << std::endl;

    // ---- save one line per run, so we can average the runs later ----
    std::string results_file = "results/phase2_runs.csv";
    bool new_file = !std::ifstream(results_file).good();
    std::ofstream out(results_file, std::ios::app);
    if (!out.is_open())
    {
        std::cout << "(could not write " << results_file << ", run from the phase2 folder)" << std::endl;
        return 0;
    }
    if (new_file)
    {
        out << "version,threads,trips,load_s,distance_report_s,data_mb,peak_memory_mb,lookup_first_s,lookup_settled_s\n";
    }
    out << version << "," << threads << "," << store.size() << "," << load_seconds << "," << distance_seconds << "," << data_mb << "," << memory_mb
        << "," << lookup_first_seconds << "," << lookup_settled_seconds << "\n";

    return 0;
}
