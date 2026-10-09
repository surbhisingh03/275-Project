// Phase 2, step 1: same objects as phase 1, but loading and searching use OpenMP threads.
// Number of threads: OMP_NUM_THREADS=4 ./build/taxi_threads file.csv
#include "taxi_trip.h"
#include "csv_reader.h"
#include "trip_data.h"

#include <chrono>
#include <fstream>
#include <iostream>
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
    std::string version = "step1_threads";
    if (argc >= 3 && std::string(argv[2]) == "old")
    {
        use_old_parsing = true;
        version = "step1_threads_old";
    }

    std::cout << version << " (objects + threads), threads: " << threads << std::endl;
    std::cout << "File: " << file_name << std::endl;

    // ---- load ----
    std::vector<TaxiTrip> trips;
    double start = now();
    if (!loadTrips(file_name, threads, trips))
    {
        std::cout << "Could not open " << file_name << std::endl;
        return 1;
    }
    double load_seconds = now() - start;
    double memory_mb = peakMemoryMB();

    // the trip data itself: number of trips x bytes per trip (exact, unlike the peak below)
    double data_mb = trips.size() * sizeof(TaxiTrip) / (1024.0 * 1024.0);

    std::cout << "\nTrips loaded: " << trips.size() << std::endl;
    std::cout << "Load time: " << load_seconds << " s" << std::endl;
    std::cout << "sizeof(TaxiTrip): " << sizeof(TaxiTrip) << " bytes" << std::endl;
    std::cout << "Trip data in memory: " << data_mb << " MB" << std::endl;
    std::cout << "Peak memory (whole program): " << memory_mb << " MB" << std::endl;

    // ---- search 1: distance report, five buckets ----
    // bucket i is: more than limits[i] miles, up to limits[i+1] miles.
    // The first limit is -1 (not 0) so that trips of exactly 0 miles are counted in the first bucket.
    double limits[] = {-1, 1, 3, 5, 10, 1e12};
    std::string bucket_names[] = {"0-1 mile trips", "1-3 mile trips", "3-5 mile trips",
                                  "5-10 mile trips", "Over-10-mile trips"};
    SearchResult buckets[5];

    start = now();
    for (int i = 0; i < 5; i++)
    {
        buckets[i] = searchByDistance(trips, limits[i], limits[i + 1]);
    }
    double distance_seconds = now() - start;

    std::cout << "\nDistance report" << std::endl;
    for (int i = 0; i < 5; i++)
    {
        printResult(bucket_names[i], buckets[i]);
    }
    std::cout << "Distance report time (5 searches): " << distance_seconds << " s" << std::endl;

    // ---- search 2: time windows of different sizes ----
    std::string window_names[] = {"1 hour", "1 day", "1 week", "1 month"};
    std::string window_from[] = {"2014-03-10T08:00:00", "2014-03-10T00:00:00", "2014-03-10T00:00:00", "2014-03-01T00:00:00"};
    std::string window_to[]   = {"2014-03-10T09:00:00", "2014-03-11T00:00:00", "2014-03-17T00:00:00", "2014-04-01T00:00:00"};
    double time_seconds[4];

    std::cout << "\nTime searches" << std::endl;
    for (int i = 0; i < 4; i++)
    {
        start = now();
        SearchResult r = searchByTime(trips, parseDate(window_from[i]), parseDate(window_to[i]));
        time_seconds[i] = now() - start;

        printResult(window_names[i] + " from " + window_from[i], r);
        std::cout << "  Search time: " << time_seconds[i] << " s" << std::endl;
    }

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
        out << "version,threads,trips,load_s,distance_report_s,time_1h_s,time_1d_s,time_1w_s,time_1m_s,data_mb,peak_memory_mb\n";
    }
    out << version << "," << threads << "," << trips.size() << "," << load_seconds << "," << distance_seconds << ","
        << time_seconds[0] << "," << time_seconds[1] << "," << time_seconds[2] << ","
        << time_seconds[3] << "," << data_mb << "," << memory_mb << "\n";

    return 0;
}
