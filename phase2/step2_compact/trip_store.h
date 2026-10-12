#pragma once
#include "taxi_trip.h"
#include <cstdint>
#include <string>
#include <vector>

class TripStore
{
public:
    bool load(const std::string& file_name, int threads);

    // The same search as phase 1: more than min_miles, up to and including max_miles.
    SearchResult searchByDistance(double min_miles, double max_miles) const;


    SearchResult lookupTrips(const std::vector<long long>& ids) const;

    long long size() const { return fare_cents.size(); }
    int bytesPerTrip() const;   
private:
    void addTrip(const TaxiTrip& t);     
    void reserveRows(long long rows);               
    void mergeParts(std::vector<TripStore>& parts);

    // Times as seconds since 1 Jan 2014 (fits in 4 bytes instead of 8)
    std::vector<uint32_t> pickup_time;
    std::vector<uint32_t> dropoff_time;

    std::vector<uint8_t> passenger_count;  
    std::vector<float> trip_distance;

    std::vector<float> pickup_longitude;
    std::vector<float> pickup_latitude;
    std::vector<float> dropoff_longitude;
    std::vector<float> dropoff_latitude;

   
    std::vector<uint8_t> vendor;         
    std::vector<uint8_t> store_and_fwd;   
    std::vector<uint8_t> payment;         
    std::vector<uint8_t> rate_code;

    std::vector<int32_t> fare_cents;
    std::vector<int32_t> mta_tax_cents;
    std::vector<int32_t> tip_cents;
    std::vector<int32_t> tolls_cents;
    std::vector<int32_t> total_cents;
    std::vector<int32_t> surcharge_cents;
};
