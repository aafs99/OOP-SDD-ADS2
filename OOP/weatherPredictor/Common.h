#ifndef COMMON_H
#define COMMON_H

#include <string>

// Centralized constants
namespace Constants {
    const int MIN_YEAR = 1980;
    const int MAX_YEAR = 2019;
    const std::string DATE_FORMAT = "YYYY-MM-DD";
    const std::string DATETIME_FORMAT = "YYYY-MM-DD HH:MM:SS";
}

// Common enums used across the application
enum class TimeFrame { 
    Daily, 
    Monthly, 
    Yearly 
};

enum class FilterType {
    DateRange = 1,
    TemperatureRange = 2,
    Trend = 3,
    Volatility = 4
};
// Utility functions
namespace Utils {
    inline std::string timeFrameToString(TimeFrame timeframe) {
        switch (timeframe) {
            case TimeFrame::Yearly:  return "Yearly";
            case TimeFrame::Monthly: return "Monthly";
            case TimeFrame::Daily:   return "Daily";
            default: return "Unknown";
        }
    }
}

#endif