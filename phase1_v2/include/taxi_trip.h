#pragma once

// One taxi trip. Only the four numbers the distance report and the trip lookup use, plus pickup and dropoff time, are kept,
// so all 71.7 million trips of the full file fit in memory (32 bytes each).

struct TaxiTrip
{
    long long pickup_time = 0;      // seconds since 1 Jan 1970
    long long dropoff_time = 0;
    double trip_distance = 0;
    double fare_amount = 0;
    double tip_amount = 0;
    double total_amount = 0;
};
