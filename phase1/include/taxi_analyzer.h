#pragma once

#include "taxi_trip.h"

class TaxiAnalyzer
{
public:
    void addTrip(const TaxiTrip& trip);
    void printResults() const;

private:
    int total_count = 0;
    double total_fare = 0.0;

    int short_trip_count = 0;
    double short_trip_fare = 0.0;
    double short_trip_distance = 0.0;
    double short_trip_tip = 0.0;
    double short_trip_total = 0.0;

    int one_to_three_count = 0;
    double one_to_three_fare = 0.0;
    double one_to_three_distance = 0.0;
    double one_to_three_tip = 0.0;
    double one_to_three_total = 0.0;

    int three_to_five_count = 0;
    double three_to_five_fare = 0.0;
    double three_to_five_distance = 0.0;
    double three_to_five_tip = 0.0;
    double three_to_five_total = 0.0;

    int five_to_ten_count = 0;
    double five_to_ten_fare = 0.0;
    double five_to_ten_distance = 0.0;
    double five_to_ten_tip = 0.0;
    double five_to_ten_total = 0.0;

    int over_ten_count = 0;
    double over_ten_fare = 0.0;
    double over_ten_distance = 0.0;
    double over_ten_tip = 0.0;
    double over_ten_total = 0.0;
};