#pragma once
#include "taxi_trip.h"
#include <cstdint>
#include <string>
#include <vector>

// Step 2: all trips stored as ONE ARRAY PER FIELD, each using the smallest type that fits.
//
//   phase 1:  trips[i].trip_distance   (one 216-byte object per trip)
//   step 2:   trip_distance[i]         (one small array per field)
//
// This class is built as its own library (taxistore). main.cpp only calls the public
// functions below; the arrays are private, so we can change them without touching main.
class TripStore
{
public:
    bool load(const std::string& file_name, int threads);

    // The same search as phase 1: more than min_miles, up to and including max_miles.
    SearchResult searchByDistance(double min_miles, double max_miles) const;

    // The second query: look up the trips with these IDs (row numbers) and read ALL their fields.
    // Here one trip is spread over 18 arrays, so each lookup visits 18 places in memory.
    SearchResult lookupTrips(const std::vector<long long>& ids) const;

    long long size() const { return fare_cents.size(); }
    int bytesPerTrip() const;    // how many bytes one trip takes in our arrays

private:
    void addTrip(const TaxiTrip& t);     // convert one trip to the small types and add it
    void reserveRows(long long rows);               // make room for this many trips
    void mergeParts(std::vector<TripStore>& parts); // join the threads' stores into this one

    // Times as seconds since 1 Jan 2014 (fits in 4 bytes instead of 8)
    std::vector<uint32_t> pickup_time;
    std::vector<uint32_t> dropoff_time;

    std::vector<uint8_t> passenger_count;   // 0-255
    std::vector<float> trip_distance;

    std::vector<float> pickup_longitude;
    std::vector<float> pickup_latitude;
    std::vector<float> dropoff_longitude;
    std::vector<float> dropoff_latitude;

    // Text columns with only a few possible values become small codes (see trip_store.cpp)
    std::vector<uint8_t> vendor;          // 0 = CMT, 1 = VTS, 2 = other
    std::vector<uint8_t> store_and_fwd;   // 0 = N, 1 = Y, 2 = empty
    std::vector<uint8_t> payment;         // 0 = CRD, 1 = CSH, 2 = NOC, 3 = DIS, 4 = UNK, 5 = other
    std::vector<uint8_t> rate_code;

    // Money as whole cents: $12.50 -> 1250
    std::vector<int32_t> fare_cents;
    std::vector<int32_t> mta_tax_cents;
    std::vector<int32_t> tip_cents;
    std::vector<int32_t> tolls_cents;
    std::vector<int32_t> total_cents;
    std::vector<int32_t> surcharge_cents;
};
