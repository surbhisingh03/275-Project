#pragma once
#include "taxi_trip.h"
#include <string>
#include <vector>

// Same as phase1_v2 (a vector of TaxiTrip objects), but loading and searching use threads.

// Read all trips from the CSV file into the vector, using this many threads.
// Returns false if the file could not be opened.
bool loadTrips(const std::string& file_name, int threads, std::vector<TaxiTrip>& trips);

// The two searches. Both look at every trip, split between the threads.
SearchResult searchByDistance(const std::vector<TaxiTrip>& trips, double min_miles, double max_miles);  // more than min, up to and including max
SearchResult searchByTime(const std::vector<TaxiTrip>& trips, long long from_time, long long to_time);   // pickup from "from" up to (not including) "to"
