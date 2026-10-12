#pragma once

// One taxi trip. The four numbers the distance report and the trip lookup use, plus pickup and dropoff time.
struct TaxiTrip
{
    long long pickup_time = 0;      // seconds since 1 Jan 1970
    long long dropoff_time = 0;
    double trip_distance = 0;
    double fare_amount = 0;
    double tip_amount = 0;
    double total_amount = 0;
};


struct SearchResult
{
    long long count = 0;
    double fare_sum = 0;
    double distance_sum = 0;
    double tip_sum = 0;
    double total_sum = 0;
};
