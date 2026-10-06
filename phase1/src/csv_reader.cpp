#include "csv_reader.h"
#include <iostream>
#include <sstream>
#include <vector>

bool CSVReader::open(std::string op)
{
    fin.open(op, std::ios::in);
    if(!fin.is_open())
    {
        return false;
    }
    std::string header;
    std::getline(fin,header);
    return true;
}

bool CSVReader::readNext(TaxiTrip& trip)
{
     std::string line;
    while(std::getline(fin, line))
    {
        std::stringstream s(line);
        std::string word;
        std::vector<std::string> fields;
        while (std::getline(s, word, ','))
        {

            if(word.size()>=2 && word.front() =='"' && word.back()=='"')
            {
            word.erase(0,1);
            word.erase(word.size()-1,1);
            }
            fields.push_back(word);

        }
        if(fields.size()<=19 ||fields[7].empty()|| fields[8].empty() ||fields[15].empty()||fields[17].empty()||fields[18].empty()||fields[19].empty()) 
        {
            continue;
        }
        int passenger_co=std::stoi(fields[7]);
        trip.passenger_count=passenger_co;
        double trip_dis=std::stod(fields[8]);
        trip.trip_distance=trip_dis;
        double fare_amt=std::stod(fields[15]);
        trip.fare_amount=fare_amt;
        double tip_amt=std::stod(fields[17]);
        trip.tip_amount=tip_amt;
        double tolls_amt=std::stod(fields[18]);
        trip.tolls_amount=tolls_amt;
        double total_amt=std::stod(fields[19]);
        trip.total_amount=total_amt;

        return true;
    }
    return false;
    
    


}



