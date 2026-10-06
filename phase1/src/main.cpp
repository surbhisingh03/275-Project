#include "csv_reader.h"
#include "taxi_analyzer.h"

#include <chrono>
#include <iostream>

int main()
{
    auto start = std::chrono::steady_clock::now();

    CSVReader reader;

    bool file = reader.open("/Users/surbhisingh/Downloads/cityofnewyork.csv");

    if (!file)
    {
        std::cout << "Opening of file failed" << std::endl;
        return 1;
    }

    std::cout << "Opening of file succeeded" << std::endl;

    TaxiTrip trip;
    TaxiAnalyzer analyzer;

    while (reader.readNext(trip))
    {
        analyzer.addTrip(trip);
    }

    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double> elapsed = end - start;

    analyzer.printResults();

    std::cout << "\nProcessing time: "<< elapsed.count()<< " seconds" << std::endl;

    return 0;
}