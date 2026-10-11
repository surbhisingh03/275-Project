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


SearchResult lookupTrips(const std::vector<TaxiTrip>& trips, const std::vector<long long>& ids)
{
    SearchResult result;
    for (size_t k = 0; k < ids.size(); k++)
    {
        const TaxiTrip& t = trips[ids[k]];
        addToResult(result, t);   // fare, distance, tip, total
        // the other 14 fields
        result.all_fields_sum += (double)t.pickup_time + t.dropoff_time + t.passenger_count
            + t.pickup_longitude + t.pickup_latitude + t.dropoff_longitude + t.dropoff_latitude
            + t.vendor_id[0] + t.store_and_fwd_flag[0] + t.payment_type[0] + t.rate_code
            + t.mta_tax + t.tolls_amount + t.imp_surcharge;
    }
    return result;
}
