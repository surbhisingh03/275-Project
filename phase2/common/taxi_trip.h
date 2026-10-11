#pragma once
#include <string>

// Same trip as phase1_v2: every column as a normal C++ type.
// Step 1 stores these. Step 2 only uses one for a moment while reading a line.
struct TaxiTrip
{
    std::string vendor_id;
    long long pickup_time = 0;      // seconds since 1 Jan 1970
    long long dropoff_time = 0;
    int passenger_count = 0;
    double trip_distance = 0;

    double pickup_longitude = 0;
    double pickup_latitude = 0;
    std::string store_and_fwd_flag;
    double dropoff_longitude = 0;
    double dropoff_latitude = 0;

    std::string payment_type;
    double fare_amount = 0;
    double mta_tax = 0;
    double tip_amount = 0;
    double tolls_amount = 0;
    double total_amount = 0;
    double imp_surcharge = 0;
    int rate_code = 0;
};

// What every search gives back: how many trips matched, plus totals.
// Averages are worked out from these when printing.
struct SearchResult
{
    long long count = 0;
    double fare_sum = 0;
    double distance_sum = 0;
    double tip_sum = 0;
    double total_sum = 0;
    double all_fields_sum = 0;   // trip lookups only: every other field added up, so the whole trip is really read
};
