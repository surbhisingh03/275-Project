#include "taxi_trip.h"
#include "csv_reader.h"
#include "trip_data.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>
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
   
    rusage_info_v4 info;
    proc_pid_rusage(getpid(), RUSAGE_INFO_V4, (rusage_info_t*)&info);
    return info.ri_lifetime_max_phys_footprint / (1024.0 * 1024.0);
#else
    rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss / 1024.0;   
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
        std::cout << "Usage: " << argv[0] << " <taxi_file.csv> [max_rows]" << std::endl;
        std::cout << "  max_rows = 0 or missing means load the whole file" << std::endl;
        return 1;
    }
    std::string file_name = argv[1];
    long long max_rows = 0;
    if (argc >= 3) max_rows = std::stoll(argv[2]);

    std::cout << "File: " << file_name << std::endl;

    std::vector<TaxiTrip> trips;
    double start = now();
    if (!loadTrips(file_name, max_rows, trips))
    {
        std::cout << "Could not open " << file_name << std::endl;
        return 1;
    }
    double load_seconds = now() - start;
    double memory_mb = peakMemoryMB();

    double data_mb = trips.size() * sizeof(TaxiTrip) / (1024.0 * 1024.0);

    std::cout << "\nTrips loaded: " << trips.size() << std::endl;
    std::cout << "Load time: " << load_seconds << " s" << std::endl;
    std::cout << "sizeof(TaxiTrip): " << sizeof(TaxiTrip) << " bytes" << std::endl;
    std::cout << "Trip data in memory: " << data_mb << " MB" << std::endl;
    std::cout << "Peak memory (whole program): " << memory_mb << " MB" << std::endl;

    
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

   
    const long long LOOKUPS = 1000000;
    std::mt19937_64 random_numbers(275);
    std::vector<long long> ids(LOOKUPS);
    for (long long k = 0; k < LOOKUPS; k++)
    {
        ids[k] = random_numbers() % trips.size();
    }

    
    SearchResult looked_up;
    double lookup_first_seconds = 0, lookup_settled_seconds = 0;
    std::cout << "\nTrip lookups (" << LOOKUPS << " random trip IDs)" << std::endl;
    for (int round = 1; round <= 5; round++)
    {
        start = now();
        looked_up = lookupTrips(trips, ids);
        double seconds = now() - start;
        if (round == 1) lookup_first_seconds = seconds;
        lookup_settled_seconds = seconds;
        std::cout << "  Round " << round << ": " << seconds << " s" << std::endl;
    }
    printResult("Trip lookups", looked_up);
    std::cout << "Lookup check: average total " << std::fixed << std::setprecision(2)
              << looked_up.total_sum / looked_up.count << std::defaultfloat << std::endl;

    // ---- save one line per run, so we can average 10 runs later ----
    std::string results_file = "results/phase1_v2_runs.csv";
    bool new_file = !std::ifstream(results_file).good();
    std::ofstream out(results_file, std::ios::app);
    if (!out.is_open())
    {
        std::cout << "(could not write " << results_file << ", run from the phase1_v2 folder)" << std::endl;
        return 0;
    }
    if (new_file)
    {
        out << "trips,load_s,distance_report_s,data_mb,peak_memory_mb,lookup_first_s,lookup_settled_s\n";
    }
    out << trips.size() << "," << load_seconds << "," << distance_seconds << "," << data_mb << "," << memory_mb
        << "," << lookup_first_seconds << "," << lookup_settled_seconds << "\n";

    return 0;
}
