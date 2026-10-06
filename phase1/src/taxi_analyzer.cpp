#include "taxi_analyzer.h"
#include <iostream>

void TaxiAnalyzer::addTrip(const TaxiTrip& trip)
{
    total_count++;
    total_fare += trip.fare_amount;

    if (trip.trip_distance <= 1.0)
    {
        short_trip_count++;
        short_trip_fare += trip.fare_amount;
        short_trip_distance += trip.trip_distance;
        short_trip_tip += trip.tip_amount;
        short_trip_total += trip.total_amount;
    }

    if (trip.trip_distance > 1.0 &&
        trip.trip_distance <= 3.0)
    {
        one_to_three_count++;
        one_to_three_fare += trip.fare_amount;
        one_to_three_distance += trip.trip_distance;
        one_to_three_tip += trip.tip_amount;
        one_to_three_total += trip.total_amount;
    }

    if (trip.trip_distance > 3.0 &&
        trip.trip_distance <= 5.0)
    {
        three_to_five_count++;
        three_to_five_fare += trip.fare_amount;
        three_to_five_distance += trip.trip_distance;
        three_to_five_tip += trip.tip_amount;
        three_to_five_total += trip.total_amount;
    }

    if (trip.trip_distance > 5.0 &&
        trip.trip_distance <= 10.0)
    {
        five_to_ten_count++;
        five_to_ten_fare += trip.fare_amount;
        five_to_ten_distance += trip.trip_distance;
        five_to_ten_tip += trip.tip_amount;
        five_to_ten_total += trip.total_amount;
    }

    if (trip.trip_distance > 10.0)
    {
        over_ten_count++;
        over_ten_fare += trip.fare_amount;
        over_ten_distance += trip.trip_distance;
        over_ten_tip += trip.tip_amount;
        over_ten_total += trip.total_amount;
    }
}
void TaxiAnalyzer::printResults() const
{
    std::cout << "Rows processed: " << total_count << std::endl;

    std::cout << "Total fare amount: "<< total_fare << std::endl;

    std::cout << "Average fare: "<< total_fare / total_count << std::endl;

    std::cout << "\n0-1 mile trips: " << short_trip_count << std::endl;

    std::cout << "Average fare: "<< short_trip_fare / short_trip_count << std::endl;

    std::cout << "Average distance: " << short_trip_distance / short_trip_count << std::endl;

    std::cout << "Fare per mile: "<< short_trip_fare / short_trip_distance << std::endl;

    std::cout << "Average tip: "<< short_trip_tip / short_trip_count << std::endl;

    std::cout << "Average total: "<< short_trip_total / short_trip_count << std::endl;

    std::cout << "\n1-3 mile trips: "<< one_to_three_count << std::endl;

    std::cout << "Average fare: "<< one_to_three_fare / one_to_three_count << std::endl;

    std::cout << "Average distance: "<< one_to_three_distance / one_to_three_count << std::endl;

    std::cout << "Fare per mile: "<< one_to_three_fare / one_to_three_distance << std::endl;

    std::cout << "Average tip: "<< one_to_three_tip / one_to_three_count << std::endl;

    std::cout << "Average total: "<< one_to_three_total / one_to_three_count << std::endl;

    std::cout << "\n3-5 mile trips: "<< three_to_five_count << std::endl;

    std::cout << "Average fare: "<< three_to_five_fare / three_to_five_count << std::endl;

    std::cout << "Average distance: "<< three_to_five_distance / three_to_five_count << std::endl;

    std::cout << "Fare per mile: "<< three_to_five_fare / three_to_five_distance << std::endl;

    std::cout << "Average tip: "<< three_to_five_tip / three_to_five_count << std::endl;

    std::cout << "Average total: "<< three_to_five_total / three_to_five_count << std::endl;

    std::cout << "\n5-10 mile trips: "<< five_to_ten_count << std::endl;

    std::cout << "Average fare: "<< five_to_ten_fare / five_to_ten_count << std::endl;

    std::cout << "Average distance: " << five_to_ten_distance / five_to_ten_count << std::endl;

    std::cout << "Fare per mile: "<< five_to_ten_fare / five_to_ten_distance << std::endl;

    std::cout << "Average tip: "<< five_to_ten_tip / five_to_ten_count << std::endl;

    std::cout << "Average total: "<< five_to_ten_total / five_to_ten_count << std::endl;

    std::cout << "\nOver-10-mile trips: "<< over_ten_count << std::endl;

    std::cout << "Average fare: "<< over_ten_fare / over_ten_count << std::endl;

    std::cout << "Average distance: "<< over_ten_distance / over_ten_count << std::endl;

    std::cout << "Fare per mile: "<< over_ten_fare / over_ten_distance << std::endl;

    std::cout << "Average tip: "<< over_ten_tip / over_ten_count << std::endl;

    std::cout << "Average total: "<< over_ten_total / over_ten_count << std::endl;
}