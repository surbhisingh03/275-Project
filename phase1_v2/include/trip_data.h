#pragma once
#include "taxi_trip.h"
#include <string>
#include <vector>

// What every search gives back: how many trips matched, plus totals.
// Averages are worked out from these when printing.
struct SearchResult
{
    long long count = 0;
    double fare_sum = 0;
    double distance_sum = 0;
    double tip_sum = 0;
    double total_sum = 0;
};

// Read trips from the CSV file into the vector. max_rows = 0 means read everything.
// Returns false if the file could not be opened.
bool loadTrips(const std::string& file_name, long long max_rows, std::vector<TaxiTrip>& trips);

// The two searches. Both look at every trip.
// The vector is passed by reference (&) so the trips are not copied.
SearchResult searchByDistance(const std::vector<TaxiTrip>& trips, double min_miles, double max_miles);  // more than min, up to and including max
SearchResult searchByTime(const std::vector<TaxiTrip>& trips, long long from_time, long long to_time);   // pickup from "from" up to (not including) "to"
