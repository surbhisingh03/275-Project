#pragma once
#include "taxi_trip.h"
#include <string>
#include <vector>

// What every search gives back: how many trips matched, plus totals.
struct SearchResult
{
    long long count = 0;
    double fare_sum = 0;
    double distance_sum = 0;
    double tip_sum = 0;
    double total_sum = 0;
};


bool loadTrips(const std::string& file_name, long long max_rows, std::vector<TaxiTrip>& trips);


SearchResult searchByDistance(const std::vector<TaxiTrip>& trips, double min_miles, double max_miles);


SearchResult lookupTrips(const std::vector<TaxiTrip>& trips, const std::vector<long long>& ids);
