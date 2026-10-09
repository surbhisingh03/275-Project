#pragma once
#include "taxi_trip.h"
#include <fstream>
#include <string>
#include <vector>

// Same idea as phase1_v2's CSVReader, but it reads only ONE PART of the file,
// so every thread can have its own reader and read its own part at the same time.
class CSVReader
{
public:
    // Read the lines that start between byte "start" and byte "end".
    // A line belongs to the part where it STARTS, so no line is read twice or missed.
    bool open(const std::string& file_name, long long start, long long end);
    bool readNext(TaxiTrip& trip);     // false when this part is finished

private:
    std::ifstream fin;
    long long position = 0;       // byte position of the next line
    long long end_position = 0;
};

// Cut the file (after the header line) into equal parts, one per thread.
// bounds[i] is where part i starts, bounds[i+1] is where it ends.
// Returns an empty vector if the file cannot be opened.
std::vector<long long> splitFile(const std::string& file_name, int parts);

// Turns "2014-01-30T08:15:00.000" into seconds since 1 Jan 1970. Returns -1 if the date is bad.
// main.cpp also uses it for the time searches.
long long parseDate(const std::string& text);

// true = parse like phase 1 (timegm, std::stod, std::stoi), false = our faster parsing (default).
// main sets it before any thread starts. "extern" means: the variable itself is in csv_reader.cpp.
extern bool use_old_parsing;
