#ifndef DATA_FILTER_H
#define DATA_FILTER_H

#include <vector>
#include <string>
#include "Candlestick.h"

namespace DataFilter {
    std::vector<Candlestick> filterByDateRange(
        const std::vector<Candlestick>& data, 
        const std::string& startDate, 
        const std::string& endDate
    );
    
    std::vector<Candlestick> filterByTemperatureRange(
        const std::vector<Candlestick>& data, 
        double minTemp, 
        double maxTemp
    );
    
    std::vector<Candlestick> filterByTrend(
        const std::vector<Candlestick>& data, 
        bool uptrend
    );
    
    std::vector<Candlestick> filterByVolatility(
        const std::vector<Candlestick>& data, 
        double minVolatility
    );
    
    void printFilterStats(size_t original, size_t filtered, const std::string& filterName);
}

#endif