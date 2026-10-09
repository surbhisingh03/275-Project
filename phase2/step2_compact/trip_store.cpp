#include "trip_store.h"
#include <cmath>
#include "csv_reader.h"
#include <omp.h>

// 1 Jan 2014 00:00:00 in seconds since 1970. Our times are stored relative to this.
const long long TIME_BASE = 1388534400;

// ---------- converting to the small types ----------

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
    return (int32_t)std::llround(dollars * 100);   // round, so 0.29 does not become 28 cents
}

uint8_t toSmallNumber(int value)
{
    if (value < 0) return 0;
    if (value > 255) return 255;
    return (uint8_t)value;
}

void TripStore::addTrip(const TaxiTrip& t)
{
    // A bad date (-1) or a date before 2014 is negative here. uint32_t cannot hold negative
    // numbers, so it wraps around to a date far in the future, which no time search ever asks for.
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

// Make room for "rows" trips in every array up front, so the arrays never have to grow
// (growing means copying everything to a bigger block). Room that is never filled is
// never touched, so it does not use real memory.
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

// Small helper: add all of "from" to the end of "to", then free "from".
// "template <typename T>" lets this one function work for every array type we have
// (uint32_t, uint8_t, float, int32_t); T stands for the type of the values in the array.
template <typename T>
void moveToEnd(std::vector<T>& to, std::vector<T>& from, long long total)
{
    if ((long long)to.capacity() < total) to.reserve(total);   // room for all trips, once
    to.insert(to.end(), from.begin(), from.end());
    from.clear();
    from.shrink_to_fit();   // give this piece's memory back right away
}

// Join the threads' stores into this one, ONE COLUMN AT A TIME: first every thread's
// pickup times, then every thread's dropoff times, and so on. Each piece is freed as soon as it
// is copied, so at any moment only one extra column exists, never a second copy of everything.
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
    return 2 * sizeof(uint32_t)    // pickup and dropoff time
         + 1 * sizeof(uint8_t)     // passengers
         + 5 * sizeof(float)       // distance + 4 coordinates
         + 4 * sizeof(uint8_t)     // vendor, flag, payment, rate code
         + 6 * sizeof(int32_t);    // money columns
}

// ---------- loading (same file splitting as step 1) ----------

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

        // A line is about 210 bytes, so this part has at most (its bytes / 150) trips.
        parts[id].reserveRows((bounds[id + 1] - bounds[id]) / 150);

        TaxiTrip trip;   // only used for a moment, then converted to small types
        while (reader.readNext(trip))
        {
            parts[id].addTrip(trip);
        }
    }   // here the program waits until every thread is finished

    mergeParts(parts);
    return true;
}

// ---------- searches ----------
// Each search only reads the arrays it needs. A distance search reads the distance and the
// three money arrays, never the times, coordinates or codes, so far less memory is read than
// in phase 1, where every 216-byte trip object passes through the CPU.
// Money is added up in whole cents (exact), then turned back into dollars at the end.

SearchResult TripStore::searchByDistance(double min_miles, double max_miles) const
{
    // Note: distances are floats now. A value right at the edge (like 2.9999999) can land on the
    // other side of the limit than in phase 1, so counts could differ very slightly. We check this.
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

SearchResult TripStore::searchByTime(long long from_time, long long to_time) const
{
    long long from = from_time - TIME_BASE;
    long long to = to_time - TIME_BASE;
    long long count = 0, fare = 0, tip = 0, total = 0;
    double distance_sum = 0;

    #pragma omp parallel for reduction(+:count, fare, tip, total, distance_sum)
    for (long long i = 0; i < size(); i++)
    {
        if (pickup_time[i] >= from && pickup_time[i] < to)
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
