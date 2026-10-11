#pragma once
#include "taxi_trip.h"
#include <string>
#include <vector>

// Same as phase1_v2 (a vector of TaxiTrip objects), but loading and searching use threads.

// Read all trips from the CSV file into the vector, using this many threads.
// Returns false if the file could not be opened.
bool loadTrips(const std::string& file_name, int threads, std::vector<TaxiTrip>& trips);

// The search: looks at every trip, split between the threads, and adds up the ones with
// more than min_miles, up to and including max_miles.
SearchResult searchByDistance(const std::vector<TaxiTrip>& trips, double min_miles, double max_miles);

// The second query: look up the trips with these IDs (row numbers) and read ALL their fields.
// The lookups are split between the threads.
SearchResult lookupTrips(const std::vector<TaxiTrip>& trips, const std::vector<long long>& ids);
