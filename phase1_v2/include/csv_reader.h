#pragma once
#include "taxi_trip.h"
#include <fstream>
#include <string>

// Reads the taxi CSV file one trip at a time.
class CSVReader
{
public:
    bool open(const std::string& file_name);
    bool readNext(TaxiTrip& trip);     // false when the file is finished

private:
    std::ifstream fin;
};

// Turns "2014-01-30T08:15:00.000" into seconds since 1 Jan 1970. Returns -1 if the date is bad.
// (Used while reading the file, to store each pickup and dropoff time as one number.)
long long parseDate(const std::string& text);
