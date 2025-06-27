#include "DataFilter.h"
#include "Common.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace DataFilter {

std::vector<Candlestick> filterByDateRange(const std::vector<Candlestick>& data, 
                                          const std::string& startDate, const std::string& endDate) {
    std::vector<Candlestick> filtered;
    
    if (data.empty()) {
        std::cout << "Warning: No data provided for date range filtering.\n";
        return filtered;
    }
    
    if (startDate.empty() || endDate.empty()) {
        std::cout << "Warning: Invalid date range provided.\n";
        return filtered;
    }
    
    if (startDate > endDate) {
        std::cout << "Warning: Start date (" << startDate << ") is after end date (" << endDate << ").\n";
        return filtered;
    }
    
    size_t originalSize = data.size();
    
    for (const auto& candlestick : data) {
        const std::string& candleDate = candlestick.getDate();
        if (candleDate >= startDate && candleDate <= endDate) {
            filtered.push_back(candlestick);
        }
    }
    
    printFilterStats(originalSize, filtered.size(), "Date range filter");
    
    if (filtered.empty()) {
        std::cout << "Warning: No data found in the specified date range.\n";
    }
    
    return filtered;
}

std::vector<Candlestick> filterByTemperatureRange(const std::vector<Candlestick>& data, 
                                                 double minTemp, double maxTemp) {
    std::vector<Candlestick> filtered;
    
    if (data.empty()) {
        std::cout << "Warning: No data provided for temperature range filtering.\n";
        return filtered;
    }
    
    if (minTemp > maxTemp) {
        std::cout << "Warning: Minimum temperature (" << minTemp 
                  << ") is greater than maximum temperature (" << maxTemp << ").\n";
        return filtered;
    }
    
    size_t originalSize = data.size();
    
    for (const auto& candlestick : data) {
        double avgTemp = candlestick.getMeanTemperature();
        if (avgTemp >= minTemp && avgTemp <= maxTemp) {
            filtered.push_back(candlestick);
        }
    }
    
    std::cout << "Temperature range filter applied: " << std::fixed << std::setprecision(1) 
              << minTemp << "°C to " << maxTemp << "°C\n";
    printFilterStats(originalSize, filtered.size(), "Temperature filter");
    
    if (filtered.empty()) {
        std::cout << "Warning: No data found in the specified temperature range.\n";
    }
    
    return filtered;
}

std::vector<Candlestick> filterByTrend(const std::vector<Candlestick>& data, bool uptrend) {
    std::vector<Candlestick> filtered;
    
    if (data.empty()) {
        std::cout << "Warning: No data provided for trend filtering.\n";
        return filtered;
    }
    
    size_t originalSize = data.size();
    
    for (const auto& candlestick : data) {
        if (candlestick.isUptrend() == uptrend) {
            filtered.push_back(candlestick);
        }
    }
    
    std::cout << "Trend filter applied: " << (uptrend ? "Upward" : "Downward") << " trends only\n";
    printFilterStats(originalSize, filtered.size(), "Trend filter");
    
    if (filtered.empty()) {
        std::cout << "Warning: No " << (uptrend ? "upward" : "downward") << " trends found in the data.\n";
    }
    
    return filtered;
}

std::vector<Candlestick> filterByVolatility(const std::vector<Candlestick>& data, double minVolatility) {
    std::vector<Candlestick> filtered;
    
    if (data.empty()) {
        std::cout << "Warning: No data provided for volatility filtering.\n";
        return filtered;
    }
    
    if (minVolatility < 0) {
        std::cout << "Warning: Minimum volatility cannot be negative. Using 0 instead.\n";
        minVolatility = 0;
    }
    
    size_t originalSize = data.size();
    
    for (const auto& candlestick : data) {
        if (candlestick.getVolatility() >= minVolatility) {
            filtered.push_back(candlestick);
        }
    }
    
    std::cout << "Volatility filter applied: Minimum " << std::fixed << std::setprecision(1) 
              << minVolatility << "°C volatility\n";
    printFilterStats(originalSize, filtered.size(), "Volatility filter");
    
    if (filtered.empty()) {
        std::cout << "Warning: No data found with volatility >= " << minVolatility << "°C.\n";
    }
    
    return filtered;
}

void printFilterStats(size_t original, size_t filtered, const std::string& filterName) {
    std::cout << filterName << " applied: " << original << " -> " << filtered << " candlesticks";
    if (original > 0) {
        double percentage = (100.0 * filtered) / original;
        std::cout << " (" << std::fixed << std::setprecision(1) << percentage << "% retained)";
    }
    std::cout << "\n";
}

} // namespace DataFilter