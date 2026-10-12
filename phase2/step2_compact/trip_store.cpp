#include "trip_store.h"
#include <cmath>
#include "csv_reader.h"
#include <omp.h>

const long long TIME_BASE = 1388534400;


uint8_t vendorCode(const std::string& s)
{
    if (s == "CMT") return 0;
    if (s == "VTS") return 1;
    return 2;
}

uint8_t flagCode(const std::string& s)
{
    if (s == "N") return 0;
    if (s == "Y") return 1;
    return 2;
}

uint8_t paymentCode(const std::string& s)
{
    if (s == "CRD") return 0;
    if (s == "CSH") return 1;
    if (s == "NOC") return 2;
    if (s == "DIS") return 3;
    if (s == "UNK") return 4;
    return 5;
}

int32_t toCents(double dollars)
{
    return (int32_t)std::llround(dollars * 100);   
}

uint8_t toSmallNumber(int value)
{
    if (value < 0) return 0;
    if (value > 255) return 255;
    return (uint8_t)value;
}

void TripStore::addTrip(const TaxiTrip& t)
{
   
    pickup_time.push_back((uint32_t)(t.pickup_time - TIME_BASE));
    dropoff_time.push_back((uint32_t)(t.dropoff_time - TIME_BASE));
    passenger_count.push_back(toSmallNumber(t.passenger_count));
    trip_distance.push_back((float)t.trip_distance);

    pickup_longitude.push_back((float)t.pickup_longitude);
    pickup_latitude.push_back((float)t.pickup_latitude);
    dropoff_longitude.push_back((float)t.dropoff_longitude);
    dropoff_latitude.push_back((float)t.dropoff_latitude);

    vendor.push_back(vendorCode(t.vendor_id));
    store_and_fwd.push_back(flagCode(t.store_and_fwd_flag));
    payment.push_back(paymentCode(t.payment_type));
    rate_code.push_back(toSmallNumber(t.rate_code));

    fare_cents.push_back(toCents(t.fare_amount));
    mta_tax_cents.push_back(toCents(t.mta_tax));
    tip_cents.push_back(toCents(t.tip_amount));
    tolls_cents.push_back(toCents(t.tolls_amount));
    total_cents.push_back(toCents(t.total_amount));
    surcharge_cents.push_back(toCents(t.imp_surcharge));
}


void TripStore::reserveRows(long long rows)
{
    pickup_time.reserve(rows);
    dropoff_time.reserve(rows);
    passenger_count.reserve(rows);
    trip_distance.reserve(rows);
    pickup_longitude.reserve(rows);
    pickup_latitude.reserve(rows);
    dropoff_longitude.reserve(rows);
    dropoff_latitude.reserve(rows);
    vendor.reserve(rows);
    store_and_fwd.reserve(rows);
    payment.reserve(rows);
    rate_code.reserve(rows);
    fare_cents.reserve(rows);
    mta_tax_cents.reserve(rows);
    tip_cents.reserve(rows);
    tolls_cents.reserve(rows);
    total_cents.reserve(rows);
    surcharge_cents.reserve(rows);
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
    for (TripStore& p : parts) moveToEnd(passenger_count, p.passenger_count, total);
    for (TripStore& p : parts) moveToEnd(trip_distance, p.trip_distance, total);
    for (TripStore& p : parts) moveToEnd(pickup_longitude, p.pickup_longitude, total);
    for (TripStore& p : parts) moveToEnd(pickup_latitude, p.pickup_latitude, total);
    for (TripStore& p : parts) moveToEnd(dropoff_longitude, p.dropoff_longitude, total);
    for (TripStore& p : parts) moveToEnd(dropoff_latitude, p.dropoff_latitude, total);
    for (TripStore& p : parts) moveToEnd(vendor, p.vendor, total);
    for (TripStore& p : parts) moveToEnd(store_and_fwd, p.store_and_fwd, total);
    for (TripStore& p : parts) moveToEnd(payment, p.payment, total);
    for (TripStore& p : parts) moveToEnd(rate_code, p.rate_code, total);
    for (TripStore& p : parts) moveToEnd(fare_cents, p.fare_cents, total);
    for (TripStore& p : parts) moveToEnd(mta_tax_cents, p.mta_tax_cents, total);
    for (TripStore& p : parts) moveToEnd(tip_cents, p.tip_cents, total);
    for (TripStore& p : parts) moveToEnd(tolls_cents, p.tolls_cents, total);
    for (TripStore& p : parts) moveToEnd(total_cents, p.total_cents, total);
    for (TripStore& p : parts) moveToEnd(surcharge_cents, p.surcharge_cents, total);
}

int TripStore::bytesPerTrip() const
{
    return 2 * sizeof(uint32_t)   
         + 1 * sizeof(uint8_t)     
         + 5 * sizeof(float)       
         + 4 * sizeof(uint8_t)     
         + 6 * sizeof(int32_t);    
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
    double distance_sum = 0, all_fields_sum = 0;

    #pragma omp parallel for reduction(+:count, fare, tip, total, distance_sum, all_fields_sum)
    for (long long k = 0; k < (long long)ids.size(); k++)
    {
        long long i = ids[k];
        count++;
        fare += fare_cents[i];
        tip += tip_cents[i];
        total += total_cents[i];
        distance_sum += trip_distance[i];
        // the other 14 fields
        all_fields_sum += (double)pickup_time[i] + dropoff_time[i] + passenger_count[i]
            + pickup_longitude[i] + pickup_latitude[i] + dropoff_longitude[i] + dropoff_latitude[i]
            + vendor[i] + store_and_fwd[i] + payment[i] + rate_code[i]
            + mta_tax_cents[i] + tolls_cents[i] + surcharge_cents[i];
    }

    SearchResult result;
    result.count = count;
    result.fare_sum = fare / 100.0;
    result.tip_sum = tip / 100.0;
    result.total_sum = total / 100.0;
    result.distance_sum = distance_sum;
    result.all_fields_sum = all_fields_sum;
    return result;
}
