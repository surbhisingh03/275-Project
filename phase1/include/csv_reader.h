    #pragma once
    #include "taxi_trip.h"
    #include <fstream>
    #include <string>   


    class CSVReader
    {
        std::fstream fin;
        public:
            bool open(std::string op);
            bool readNext(TaxiTrip& trip);
    };
    