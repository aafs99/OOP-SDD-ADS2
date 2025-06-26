#ifndef TEMPERATURE_RECORD_H
#define TEMPERATURE_RECORD_H

#include <string>

struct TemperatureRecord {
    std::string date;
    double temperature;
    
    TemperatureRecord() : temperature(0.0) {}
    TemperatureRecord(const std::string& d, double t) : date(d), temperature(t) {}
};

#endif