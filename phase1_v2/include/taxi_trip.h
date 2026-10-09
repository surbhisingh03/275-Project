#pragma once
#include <string>

// One taxi trip = one row of the CSV file.
// Every column is stored as a normal C++ type (int, double, string).
// The "extra" column is not stored because it is empty in every row.
struct TaxiTrip
{
    std::string vendor_id;          // "CMT" or "VTS"
    long long pickup_time = 0;      // seconds since 1 Jan 1970
    long long dropoff_time = 0;
    int passenger_count = 0;
    double trip_distance = 0;       // miles

    double pickup_longitude = 0;
    double pickup_latitude = 0;
    std::string store_and_fwd_flag; // "Y", "N" or empty
    double dropoff_longitude = 0;
    double dropoff_latitude = 0;

    std::string payment_type;       // "CRD", "CSH", ...
    double fare_amount = 0;
    double mta_tax = 0;
    double tip_amount = 0;
    double tolls_amount = 0;
    double total_amount = 0;
    double imp_surcharge = 0;
    int rate_code = 0;
};
