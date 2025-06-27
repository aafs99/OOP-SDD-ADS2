#include "DataFilter.h"
#include "Common.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace DataFilter {

namespace {
    // Helper function for input validation
    bool validateFilterInput(const std::vector<Candlestick>& data, const std::string& filterType) {
        if (data.empty()) {
            std::cout << "Warning: No data provided for " << filterType << " filtering.\n";
            return false;
        }
        return true;
    }
    
    // Helper function to handle empty results and print stats
    void handleFilterResults(const std::vector<Candlestick>& filtered, size_t originalSize, 
                            const std::string& filterName, const std::string& warningMessage = "") {
        printFilterStats(originalSize, filtered.size(), filterName);
        
        if (filtered.empty() && !warningMessage.empty()) {
            std::cout << "Warning: " << warningMessage << "\n";
        }
    }
    
    //Applies a filter based on a predicate function
    // and handles the results, including printing stats and warnings.
    template<typename PredicateFunc>
    std::vector<Candlestick> applyFilter(const std::vector<Candlestick>& data, 
                                        const std::string& filterType,
                                        const std::string& filterName,
                                        const std::string& warningMessage,
                                        PredicateFunc predicate) {
        std::vector<Candlestick> filtered;
        
        if (!validateFilterInput(data, filterType)) {
            return filtered;
        }
        
        const size_t originalSize = data.size();
        
        // Reserve space to avoid reallocations
        filtered.reserve(data.size());
        
        // Use copy_if for better performance
        std::copy_if(data.begin(), data.end(), std::back_inserter(filtered), predicate);
        
        handleFilterResults(filtered, originalSize, filterName, warningMessage);
        
        return filtered;
    }
}

//self written code start
std::vector<Candlestick> filterByDateRange(const std::vector<Candlestick>& data, 
                                          const std::string& startDate, const std::string& endDate) {
    std::vector<Candlestick> filtered;
    
    if (!validateFilterInput(data, "date range")) {
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
    
    // Use the template helper function
    return applyFilter(data, "date range", "Date range filter", 
                      "No data found in the specified date range.",
                      [&startDate, &endDate](const Candlestick& candlestick) {
                          const std::string& candleDate = candlestick.getDate();
                          return candleDate >= startDate && candleDate <= endDate;
                      });
}

std::vector<Candlestick> filterByTemperatureRange(const std::vector<Candlestick>& data, 
                                                 double minTemp, double maxTemp) {
    std::vector<Candlestick> filtered;
    
    if (!validateFilterInput(data, "temperature range")) {
        return filtered;
    }
    
    if (minTemp > maxTemp) {
        std::cout << "Warning: Minimum temperature (" << minTemp 
                  << ") is greater than maximum temperature (" << maxTemp << ").\n";
        return filtered;
    }
    
    std::cout << "Temperature range filter applied: " << std::fixed << std::setprecision(1) 
              << minTemp << "°C to " << maxTemp << "°C\n";
    
    // Use the template helper function
    return applyFilter(data, "temperature range", "Temperature filter",
                      "No data found in the specified temperature range.",
                      [minTemp, maxTemp](const Candlestick& candlestick) {
                          double avgTemp = candlestick.getMeanTemperature();
                          return avgTemp >= minTemp && avgTemp <= maxTemp;
                      });
}

std::vector<Candlestick> filterByTrend(const std::vector<Candlestick>& data, bool uptrend) {
    std::cout << "Trend filter applied: " << (uptrend ? "Upward" : "Downward") << " trends only\n";
    
    const std::string warningMessage = "No " + std::string(uptrend ? "upward" : "downward") + 
                                      " trends found in the data.";
    
    // Use the template helper function
    return applyFilter(data, "trend", "Trend filter", warningMessage,
                      [uptrend](const Candlestick& candlestick) {
                          return candlestick.isUptrend() == uptrend;
                      });
}

std::vector<Candlestick> filterByVolatility(const std::vector<Candlestick>& data, double minVolatility) {
    std::vector<Candlestick> filtered;
    
    if (!validateFilterInput(data, "volatility")) {
        return filtered;
    }
    
    if (minVolatility < 0) {
        std::cout << "Warning: Minimum volatility cannot be negative. Using 0 instead.\n";
        minVolatility = 0;
    }
    
    std::cout << "Volatility filter applied: Minimum " << std::fixed << std::setprecision(1) 
              << minVolatility << "°C volatility\n";
    
    const std::string warningMessage = "No data found with volatility >= " + 
                                      std::to_string(static_cast<int>(minVolatility)) + "°C.";
    
    // Use the template helper function
    return applyFilter(data, "volatility", "Volatility filter", warningMessage,
                      [minVolatility](const Candlestick& candlestick) {
                          return candlestick.getVolatility() >= minVolatility;
                      });
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
//self written code end