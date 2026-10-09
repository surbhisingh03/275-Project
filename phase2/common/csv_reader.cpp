#include "csv_reader.h"
#include <charconv>
#include <cstdio>
#include <ctime>

// The file has 19 columns, always in this order:
//  0 vendor_id          1 pickup_datetime    2 dropoff_datetime   3 passenger_count
//  4 trip_distance      5 pickup_longitude   6 pickup_latitude    7 store_and_fwd_flag
//  8 dropoff_longitude  9 dropoff_latitude  10 payment_type      11 fare_amount
// 12 mta_tax           13 tip_amount        14 tolls_amount      15 total_amount
// 16 imp_surcharge     17 extra (always empty, not stored)       18 rate_code
const int COLUMN_COUNT = 19;

bool use_old_parsing = false;

// Split one line into fields at each comma.
// Every field in the file is in quotes ("VTS"), so the quote characters are skipped.
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
    fields.push_back(current);   // last field
    return fields;
}

// ---------- dates ----------
// Phase 1 used timegm() to turn a date into seconds. We found it is very slow, and on macOS it
// gets much SLOWER with more threads (every call waits for a shared lock). So we count the
// seconds ourselves: days in the full years before, days in the months before, then the time.

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
    if (month > 2 && isLeapYear(year)) days += 1;   // 29 February
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
    if (month < 1 || month > 12) return -1;   // days_before_month[month - 1] needs a real month

    if (use_old_parsing)
    {
        std::tm t = {};
        t.tm_year = year - 1900;   // tm counts years from 1900
        t.tm_mon = month - 1;      // and months from 0
        t.tm_mday = day;
        t.tm_hour = hour;
        t.tm_min = minute;
        t.tm_sec = second;
        return timegm(&t);
    }
    return secondsSince1970(year, month, day, hour, minute, second);
}

// ---------- numbers ----------
// Phase 1 used std::stod / std::stoi. On macOS these also slow down a bit with threads
// (they check a shared "locale" setting). std::from_chars does not, and is faster anyway.
// from_chars reads the characters from the first pointer up to the second one.
// It never throws: if the text is not a number, value just stays 0.

double toDouble(const std::string& s)
{
    if (s.empty()) return 0;
    if (use_old_parsing)
    {
        // std::stod throws an error for text that is not a number ("abc") or is too big.
        // Use 0 instead, like from_chars below, so one bad value cannot stop the whole load.
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

int toInt(const std::string& s)
{
    if (s.empty()) return 0;
    if (use_old_parsing)
    {
        try
        {
            return std::stoi(s);
        }
        catch (...)
        {
            return 0;
        }
    }

    int value = 0;
    std::from_chars(s.data(), s.data() + s.size(), value);
    return value;
}

// ---------- splitting the file between threads ----------

std::vector<long long> splitFile(const std::string& file_name, int parts)
{
    std::vector<long long> bounds;
    std::ifstream fin(file_name, std::ios::binary);
    if (!fin.is_open()) return bounds;

    // the trips start right after the header line and its newline
    std::string header;
    std::getline(fin, header);
    long long data_start = header.size() + 1;

    // file size = position of the end of the file
    fin.seekg(0, std::ios::end);
    long long file_size = fin.tellg();

    long long part_size = (file_size - data_start) / parts;
    for (int i = 0; i < parts; i++)
    {
        bounds.push_back(data_start + i * part_size);
    }
    bounds.push_back(file_size);   // the last part goes to the end of the file
    return bounds;
}

bool CSVReader::open(const std::string& file_name, long long start, long long end)
{
    fin.open(file_name, std::ios::binary);
    if (!fin.is_open()) return false;
    end_position = end;

    // Our start is probably in the middle of a line. That line belongs to the
    // part before us, so skip ahead to the next newline.
    // (We look from start - 1: if that byte is a newline, start is already a line start.)
    fin.seekg(start - 1);
    std::string partial_line;
    std::getline(fin, partial_line);
    position = start - 1 + partial_line.size() + 1;
    return true;
}

bool CSVReader::readNext(TaxiTrip& trip)
{
    std::string line;
    while (position < end_position && std::getline(fin, line))   // stop when the next line starts in the next part
    {
        position += line.size() + 1;
        std::vector<std::string> f = splitLine(line);

        // a broken line (wrong number of columns) would make f[...] read past the end, so skip it
        if (f.size() != COLUMN_COUNT) continue;

        trip.vendor_id          = f[0];
        trip.pickup_time        = parseDate(f[1]);
        trip.dropoff_time       = parseDate(f[2]);
        trip.passenger_count    = toInt(f[3]);
        trip.trip_distance      = toDouble(f[4]);
        trip.pickup_longitude   = toDouble(f[5]);
        trip.pickup_latitude    = toDouble(f[6]);
        trip.store_and_fwd_flag = f[7];
        trip.dropoff_longitude  = toDouble(f[8]);
        trip.dropoff_latitude   = toDouble(f[9]);
        trip.payment_type       = f[10];
        trip.fare_amount        = toDouble(f[11]);
        trip.mta_tax            = toDouble(f[12]);
        trip.tip_amount         = toDouble(f[13]);
        trip.tolls_amount       = toDouble(f[14]);
        trip.total_amount       = toDouble(f[15]);
        trip.imp_surcharge      = toDouble(f[16]);
        trip.rate_code          = toInt(f[18]);
        return true;
    }
    return false;
}
