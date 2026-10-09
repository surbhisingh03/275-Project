#include "trip_data.h"
#include "csv_reader.h"

bool loadTrips(const std::string& file_name, long long max_rows, std::vector<TaxiTrip>& trips)
{
    CSVReader reader;
    if (!reader.open(file_name)) return false;

    TaxiTrip trip;
    while (reader.readNext(trip))
    {
        trips.push_back(trip);
        if (max_rows > 0 && trips.size() >= max_rows) break;
    }
    return true;
}

// add one matching trip to a search result
void addToResult(SearchResult& result, const TaxiTrip& t)
{
    result.count++;
    result.fare_sum += t.fare_amount;
    result.distance_sum += t.trip_distance;
    result.tip_sum += t.tip_amount;
    result.total_sum += t.total_amount;
}

SearchResult searchByDistance(const std::vector<TaxiTrip>& trips, double min_miles, double max_miles)
{
    SearchResult result;
    for (size_t i = 0; i < trips.size(); i++)
    {
        if (trips[i].trip_distance > min_miles && trips[i].trip_distance <= max_miles)
        {
            addToResult(result, trips[i]);
        }
    }
    return result;
}

SearchResult searchByTime(const std::vector<TaxiTrip>& trips, long long from_time, long long to_time)
{
    SearchResult result;
    for (size_t i = 0; i < trips.size(); i++)
    {
        if (trips[i].pickup_time >= from_time && trips[i].pickup_time < to_time)
        {
            addToResult(result, trips[i]);
        }
    }
    return result;
}
