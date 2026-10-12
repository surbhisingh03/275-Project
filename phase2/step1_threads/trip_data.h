#pragma once
#include "taxi_trip.h"
#include <string>
#include <vector>


bool loadTrips(const std::string& file_name, int threads, std::vector<TaxiTrip>& trips);


SearchResult searchByDistance(const std::vector<TaxiTrip>& trips, double min_miles, double max_miles);


SearchResult lookupTrips(const std::vector<TaxiTrip>& trips, const std::vector<long long>& ids);
