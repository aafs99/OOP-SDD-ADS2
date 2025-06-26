#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include <string>
#include <vector>
#include "TemperatureRecord.h"

namespace DataLoader {
    std::vector<TemperatureRecord> loadCSV(
        const std::string& filename, 
        const std::string& countryCode, 
        int startYear, 
        int endYear
    );
}

#endif