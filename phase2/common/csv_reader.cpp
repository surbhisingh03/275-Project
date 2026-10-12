#include "csv_reader.h"
#include <charconv>
#include <cstdio>
#include <ctime>


const int COLUMN_COUNT = 19;

bool use_old_parsing = false;


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

bool isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

long long secondsSince1970(int year, int month, int day, int hour, int minute, int second)
{
    int days_before_month[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    long long days = 0;
    for (int y = 1970; y < year; y++)
    {
        if (isLeapYear(y)) days += 366;
        else days += 365;
    }
    days += days_before_month[month - 1];
    if (month > 2 && isLeapYear(year)) days += 1;   
    days += day - 1;

    return days * 86400 + hour * 3600 + minute * 60 + second;
}

long long parseDate(const std::string& text)
{
    int year, month, day, hour, minute, second;
    if (std::sscanf(text.c_str(), "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second) != 6)
    {
        return -1;
    }
    if (month < 1 || month > 12) return -1;   

    if (use_old_parsing)
    {
        std::tm t = {};
        t.tm_year = year - 1900;   
        t.tm_mon = month - 1;      
        t.tm_mday = day;
        t.tm_hour = hour;
        t.tm_min = minute;
        t.tm_sec = second;
        return timegm(&t);
    }
    return secondsSince1970(year, month, day, hour, minute, second);
}



double toDouble(const std::string& s)
{
    if (s.empty()) return 0;
    if (use_old_parsing)
    {
       
        try
        {
            return std::stod(s);
        }
        catch (...)
        {
            return 0;
        }
    }

    double value = 0;
    std::from_chars(s.data(), s.data() + s.size(), value);
    return value;
}


std::vector<long long> splitFile(const std::string& file_name, int parts)
{
    std::vector<long long> bounds;
    std::ifstream fin(file_name, std::ios::binary);
    if (!fin.is_open()) return bounds;

    std::string header;
    std::getline(fin, header);
    long long data_start = header.size() + 1;

    fin.seekg(0, std::ios::end);
    long long file_size = fin.tellg();

    long long part_size = (file_size - data_start) / parts;
    for (int i = 0; i < parts; i++)
    {
        bounds.push_back(data_start + i * part_size);
    }
    bounds.push_back(file_size);  
    return bounds;
}

bool CSVReader::open(const std::string& file_name, long long start, long long end)
{
    fin.open(file_name, std::ios::binary);
    if (!fin.is_open()) return false;
    end_position = end;


    fin.seekg(start - 1);
    std::string partial_line;
    std::getline(fin, partial_line);
    position = start - 1 + partial_line.size() + 1;
    return true;
}

bool CSVReader::readNext(TaxiTrip& trip)
{
    std::string line;
    while (position < end_position && std::getline(fin, line))   
    {
        position += line.size() + 1;
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
