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

// The search: looks at every trip and adds up the ones with
// more than min_miles, up to and including max_miles.
// The vector is passed by reference (&) so the trips are not copied.
SearchResult searchByDistance(const std::vector<TaxiTrip>& trips, double min_miles, double max_miles);
