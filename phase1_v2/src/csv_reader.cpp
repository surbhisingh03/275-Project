#include "csv_reader.h"
#include <cstdio>
#include <ctime>
#include <vector>


const int COLUMN_COUNT = 19;


std::vector<std::string> splitLine(const std::string& line)
{
    std::vector<std::string> fields;
    std::string current;

    for (char c : line)
    {
        if (c == ',')
        {
            fields.push_back(current);
            current = "";
        }
        else if (c != '"' && c != '\r')
        {
            current += c;
        }
    }
    fields.push_back(current);   
    return fields;
}

long long parseDate(const std::string& text)
{
    int year, month, day, hour, minute, second;
    if (std::sscanf(text.c_str(), "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second) != 6)
    {
        return -1;
    }

    std::tm t = {};
    t.tm_year = year - 1900;   
    t.tm_mon = month - 1;      
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;
    return timegm(&t);  
}


double toDouble(const std::string& s)
{
    if (s.empty()) return 0;
    try
    {
        return std::stod(s);
    }
    catch (...)
    {
        return 0;
    }
}

bool CSVReader::open(const std::string& file_name)
{
    fin.open(file_name);
    if (!fin.is_open()) return false;

    std::string header;
    std::getline(fin, header);   
    return true;
}

bool CSVReader::readNext(TaxiTrip& trip)
{
    std::string line;
    while (std::getline(fin, line))
    {
        std::vector<std::string> f = splitLine(line);

        if (f.size() != COLUMN_COUNT) continue;

        trip.pickup_time   = parseDate(f[1]);
        trip.dropoff_time  = parseDate(f[2]);
        trip.trip_distance = toDouble(f[4]);
        trip.fare_amount   = toDouble(f[11]);
        trip.tip_amount    = toDouble(f[13]);
        trip.total_amount  = toDouble(f[15]);
        return true;
    }
    return false;
}
