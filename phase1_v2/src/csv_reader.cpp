#include "csv_reader.h"
#include <cstdio>
#include <ctime>
#include <vector>

// The file has 19 columns, always in this order:
//  0 vendor_id          1 pickup_datetime    2 dropoff_datetime   3 passenger_count
//  4 trip_distance      5 pickup_longitude   6 pickup_latitude    7 store_and_fwd_flag
//  8 dropoff_longitude  9 dropoff_latitude  10 payment_type      11 fare_amount
// 12 mta_tax           13 tip_amount        14 tolls_amount      15 total_amount
// 16 imp_surcharge     17 extra (always empty, not stored)       18 rate_code
const int COLUMN_COUNT = 19;

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

long long parseDate(const std::string& text)
{
    int year, month, day, hour, minute, second;
    if (std::sscanf(text.c_str(), "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second) != 6)
    {
        return -1;
    }

    std::tm t = {};
    t.tm_year = year - 1900;   // tm counts years from 1900
    t.tm_mon = month - 1;      // and months from 0
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;
    return timegm(&t);   // treat the time as UTC so the result does not depend on the laptop's time zone
}

// Empty field -> 0, otherwise convert the text to a number.
// std::stod / std::stoi throw an error if the text is not a number ("abc") or is too big.
// We catch that and use 0, so one bad value cannot stop the whole load. The trip is still kept,
// the same as phase 2's from_chars does, so both phases load the same number of trips.
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

int toInt(const std::string& s)
{
    if (s.empty()) return 0;
    try
    {
        return std::stoi(s);
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
    std::getline(fin, header);   // skip the header line
    return true;
}

bool CSVReader::readNext(TaxiTrip& trip)
{
    std::string line;
    while (std::getline(fin, line))
    {
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
