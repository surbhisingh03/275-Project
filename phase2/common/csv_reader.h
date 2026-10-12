#pragma once
#include "taxi_trip.h"
#include <fstream>
#include <string>
#include <vector>


class CSVReader
{
public:
   
    bool open(const std::string& file_name, long long start, long long end);
    bool readNext(TaxiTrip& trip);     
private:
    std::ifstream fin;
    long long position = 0;      
    long long end_position = 0;
};


std::vector<long long> splitFile(const std::string& file_name, int parts);


long long parseDate(const std::string& text);


extern bool use_old_parsing;
