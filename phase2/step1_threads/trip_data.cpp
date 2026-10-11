#include "trip_data.h"
#include "csv_reader.h"
#include <omp.h>

bool loadTrips(const std::string& file_name, int threads, std::vector<TaxiTrip>& trips)
{
    std::vector<long long> bounds = splitFile(file_name, threads);
    if (bounds.empty()) return false;

    // every thread fills its own vector, so threads never write to the same place
    std::vector<std::vector<TaxiTrip>> parts(threads);

    // Everything inside { } runs on every thread at the same time.
    // Variables made inside (reader, trip) are private: each thread has its own.
    #pragma omp parallel num_threads(threads)
    {
        int id = omp_get_thread_num();   // 0, 1, 2, ...

        CSVReader reader;
        reader.open(file_name, bounds[id], bounds[id + 1]);

        // Make room up front so the vector never has to grow (growing copies everything).
        // A line is about 210 bytes, so this part has at most (its bytes / 150) trips.
        // Room that is never filled is never touched, so it does not use real memory.
        parts[id].reserve((bounds[id + 1] - bounds[id]) / 150);

        TaxiTrip trip;
        while (reader.readNext(trip))
        {
            parts[id].push_back(trip);
        }
    }   // here the program waits until every thread is finished

    // join the parts into one vector (in file order), freeing each part as we go
    long long total = 0;
    for (int i = 0; i < threads; i++) total += parts[i].size();
    trips.reserve(total);

    for (int i = 0; i < threads; i++)
    {
        trips.insert(trips.end(), parts[i].begin(), parts[i].end());
        parts[i].clear();
        parts[i].shrink_to_fit();   // give the memory back now
    }
    return true;
}

// For the searches each thread checks a share of the trips and keeps its own totals.
// reduction(+: ...) gives every thread its own copy of these variables, and adds the
// copies together at the end. (It needs plain variables, so we fill SearchResult afterwards.)

SearchResult searchByDistance(const std::vector<TaxiTrip>& trips, double min_miles, double max_miles)
{
    long long count = 0;
    double fare_sum = 0, distance_sum = 0, tip_sum = 0, total_sum = 0;

    #pragma omp parallel for reduction(+:count, fare_sum, distance_sum, tip_sum, total_sum)
    for (long long i = 0; i < (long long)trips.size(); i++)
    {
        if (trips[i].trip_distance > min_miles && trips[i].trip_distance <= max_miles)
        {
            count++;
            fare_sum += trips[i].fare_amount;
            distance_sum += trips[i].trip_distance;
            tip_sum += trips[i].tip_amount;
            total_sum += trips[i].total_amount;
        }
    }

    SearchResult result;
    result.count = count;
    result.fare_sum = fare_sum;
    result.distance_sum = distance_sum;
    result.tip_sum = tip_sum;
    result.total_sum = total_sum;
    return result;
}


SearchResult lookupTrips(const std::vector<TaxiTrip>& trips, const std::vector<long long>& ids)
{
    long long count = 0;
    double fare_sum = 0, distance_sum = 0, tip_sum = 0, total_sum = 0, all_fields_sum = 0;

    #pragma omp parallel for reduction(+:count, fare_sum, distance_sum, tip_sum, total_sum, all_fields_sum)
    for (long long k = 0; k < (long long)ids.size(); k++)
    {
        const TaxiTrip& t = trips[ids[k]];
        count++;
        fare_sum += t.fare_amount;
        distance_sum += t.trip_distance;
        tip_sum += t.tip_amount;
        total_sum += t.total_amount;
        // the other 14 fields
        all_fields_sum += (double)t.pickup_time + t.dropoff_time + t.passenger_count
            + t.pickup_longitude + t.pickup_latitude + t.dropoff_longitude + t.dropoff_latitude
            + t.vendor_id[0] + t.store_and_fwd_flag[0] + t.payment_type[0] + t.rate_code
            + t.mta_tax + t.tolls_amount + t.imp_surcharge;
    }

    SearchResult result;
    result.count = count;
    result.fare_sum = fare_sum;
    result.distance_sum = distance_sum;
    result.tip_sum = tip_sum;
    result.total_sum = total_sum;
    result.all_fields_sum = all_fields_sum;
    return result;
}
