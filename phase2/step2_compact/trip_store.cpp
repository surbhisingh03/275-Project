#include "trip_store.h"
#include <cmath>
#include "csv_reader.h"
#include <omp.h>

const long long TIME_BASE = 1388534400;

int32_t toCents(double dollars)
{
    return (int32_t)std::llround(dollars * 100);   
}

void TripStore::addTrip(const TaxiTrip& t)
{
    pickup_time.push_back((uint32_t)(t.pickup_time - TIME_BASE));
    dropoff_time.push_back((uint32_t)(t.dropoff_time - TIME_BASE));
    trip_distance.push_back((float)t.trip_distance);
    fare_cents.push_back(toCents(t.fare_amount));
    tip_cents.push_back(toCents(t.tip_amount));
    total_cents.push_back(toCents(t.total_amount));
}


void TripStore::reserveRows(long long rows)
{
    pickup_time.reserve(rows);
    dropoff_time.reserve(rows);
    trip_distance.reserve(rows);
    fare_cents.reserve(rows);
    tip_cents.reserve(rows);
    total_cents.reserve(rows);
}


template <typename T>
void moveToEnd(std::vector<T>& to, std::vector<T>& from, long long total)
{
    if ((long long)to.capacity() < total) to.reserve(total);   // room for all trips, once
    to.insert(to.end(), from.begin(), from.end());
    from.clear();
    from.shrink_to_fit();   
}


void TripStore::mergeParts(std::vector<TripStore>& parts)
{
    long long total = 0;
    for (TripStore& p : parts) total += p.size();

    for (TripStore& p : parts) moveToEnd(pickup_time, p.pickup_time, total);
    for (TripStore& p : parts) moveToEnd(dropoff_time, p.dropoff_time, total);
    for (TripStore& p : parts) moveToEnd(trip_distance, p.trip_distance, total);
    for (TripStore& p : parts) moveToEnd(fare_cents, p.fare_cents, total);
    for (TripStore& p : parts) moveToEnd(tip_cents, p.tip_cents, total);
    for (TripStore& p : parts) moveToEnd(total_cents, p.total_cents, total);
}

int TripStore::bytesPerTrip() const
{
    return 2 * sizeof(uint32_t)   // pickup and dropoff time
         + sizeof(float)          // trip_distance
         + 3 * sizeof(int32_t);   // fare, tip, total
}


bool TripStore::load(const std::string& file_name, int threads)
{
    std::vector<long long> bounds = splitFile(file_name, threads);
    if (bounds.empty()) return false;

    // each thread fills its own small store
    std::vector<TripStore> parts(threads);

    #pragma omp parallel num_threads(threads)
    {
        int id = omp_get_thread_num();

        CSVReader reader;
        reader.open(file_name, bounds[id], bounds[id + 1]);

        parts[id].reserveRows((bounds[id + 1] - bounds[id]) / 150);

        TaxiTrip trip;  
        while (reader.readNext(trip))
        {
            parts[id].addTrip(trip);
        }
    }   

    mergeParts(parts);
    return true;
}


SearchResult TripStore::searchByDistance(double min_miles, double max_miles) const
{
    float low = (float)min_miles;
    float high = (float)max_miles;
    long long count = 0, fare = 0, tip = 0, total = 0;
    double distance_sum = 0;

    #pragma omp parallel for reduction(+:count, fare, tip, total, distance_sum)
    for (long long i = 0; i < size(); i++)
    {
        if (trip_distance[i] > low && trip_distance[i] <= high)
        {
            count++;
            fare += fare_cents[i];
            tip += tip_cents[i];
            total += total_cents[i];
            distance_sum += trip_distance[i];
        }
    }

    SearchResult result;
    result.count = count;
    result.fare_sum = fare / 100.0;
    result.tip_sum = tip / 100.0;
    result.total_sum = total / 100.0;
    result.distance_sum = distance_sum;
    return result;
}


SearchResult TripStore::lookupTrips(const std::vector<long long>& ids) const
{
    long long count = 0, fare = 0, tip = 0, total = 0;
    double distance_sum = 0;

    #pragma omp parallel for reduction(+:count, fare, tip, total, distance_sum)
    for (long long k = 0; k < (long long)ids.size(); k++)
    {
        long long i = ids[k];
        count++;
        fare += fare_cents[i];
        tip += tip_cents[i];
        total += total_cents[i];
        distance_sum += trip_distance[i];
    }

    SearchResult result;
    result.count = count;
    result.fare_sum = fare / 100.0;
    result.tip_sum = tip / 100.0;
    result.total_sum = total / 100.0;
    result.distance_sum = distance_sum;
    return result;
}
