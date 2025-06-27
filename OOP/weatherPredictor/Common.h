#ifndef COMMON_H
#define COMMON_H

#include <string>

// Centralized constants
namespace Constants {
    const int MIN_YEAR = 1980;
    const int MAX_YEAR = 2019;
    const std::string DATE_FORMAT = "YYYY-MM-DD";
    const std::string DATETIME_FORMAT = "YYYY-MM-DD HH:MM:SS";
    
    // FIXED: Added constants to eliminate magic numbers
    const double EPSILON = 1e-10;                    // For floating point comparisons
    const double CHART_PADDING_RATIO = 0.1;         // 10% padding for chart display
    const int CHART_WIDTH = 20;                     // Width of prediction comparison charts
    const double STABILITY_SCALE_FACTOR = 5.0;      // Scaling factor for stability confidence
    const double HIGH_CONFIDENCE_THRESHOLD = 0.7;   // Threshold for high confidence
    const double MODERATE_CONFIDENCE_THRESHOLD = 0.4; // Threshold for moderate confidence
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