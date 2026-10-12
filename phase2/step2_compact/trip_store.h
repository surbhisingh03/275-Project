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

    // The four fields both queries use, each stored in a smaller type.
    std::vector<float> trip_distance;
    std::vector<int32_t> fare_cents;
    std::vector<int32_t> tip_cents;
    std::vector<int32_t> total_cents;
};
