#ifndef COMMON_H
#define COMMON_H

#include <string>

// Centralized constants
namespace Constants {
    const int MIN_YEAR = 1980;
    const int MAX_YEAR = 2019;
    const std::string DATE_FORMAT = "YYYY-MM-DD";
    const std::string DATETIME_FORMAT = "YYYY-MM-DD HH:MM:SS";
    
    // Added constants to eliminate magic numbers
    const double EPSILON = 1e-10;                    // For floating point comparisons
    const double CHART_PADDING_RATIO = 0.1;         // 10% padding for chart display
    const int CHART_WIDTH = 20;                     // Width of prediction comparison charts
    const double STABILITY_SCALE_FACTOR = 5.0;      // Scaling factor for stability confidence
    const double HIGH_CONFIDENCE_THRESHOLD = 0.7;   // Threshold for high confidence
    const double MODERATE_CONFIDENCE_THRESHOLD = 0.4; // Threshold for moderate confidence
    
    // Chart display constants
    const int Y_AXIS_WIDTH = 8;                     // Width of Y-axis labels
    const int MAX_CHART_WIDTH = 120;                // Maximum chart width
    const size_t PREFERRED_MAX_DISPLAY_POINTS = 60; // Preferred maximum data points to display
    const size_t MODERATE_MAX_DISPLAY_POINTS = 100; // Moderate maximum data points
    const size_t ABSOLUTE_MAX_DISPLAY_POINTS = 120; // Absolute maximum data points
    const int MIN_CHARS_PER_LABEL = 6;              // Minimum characters per X-axis label
    
    // ENHANCED: New chart constants to eliminate magic numbers
    const int ENHANCED_CHART_MIN_WIDTH = 50;        // Minimum enhanced chart width
    const int ENHANCED_CHART_MAX_WIDTH = 80;        // Maximum enhanced chart width
    const int ENHANCED_CHART_DEFAULT_WIDTH = 60;    // Default enhanced chart width
    const double ENHANCED_CHART_PADDING_RATIO = 0.15; // Enhanced chart padding (15%)
    const double MIN_CHART_PADDING = 1.0;           // Minimum padding in temperature units
    const int VERTICAL_CHART_HEIGHT = 20;           // Default vertical chart height
    const int VERTICAL_CHART_MAX_PERIODS = 40;      // Maximum periods shown in vertical chart
    const int PERIOD_LABEL_INTERVAL = 5;            // Interval for period labels in vertical chart
    const double TEMP_TOLERANCE_FACTOR = 0.5;       // Temperature tolerance for symbol placement
    const int PREDICTION_VALUE_WIDTH = 6;           // Width for prediction value display
    const double TEMP_RANGE_SANITY_CHECK = 100.0;   // Sanity check for temperature values
    
    // Prediction model constants
    const size_t MIN_LINEAR_DATA_SIZE = 2;          // Minimum data points for linear regression
    const size_t MIN_HEURISTIC_DATA_SIZE = 2;       // Minimum data points for heuristic model
    const size_t MIN_MOVING_AVERAGE_DATA_SIZE = 1;  // Minimum data points for moving average
    
    // Table formatting constants
    const int TABLE_SEPARATOR_WIDTH = 60;           // Width for table separators
    const int DATE_COLUMN_WIDTH = 12;               // Width for date column
    const int TEMP_COLUMN_WIDTH = 8;                // Width for temperature columns
    const int CHANGE_COLUMN_WIDTH = 6;              // Width for change column
    
    // Section separator widths
    const int SECTION_SEPARATOR_WIDTH_60 = 60;      // Standard section separator
    const int ANALYSIS_SEPARATOR_WIDTH_65 = 65;     // Analysis section separator
    const int VALIDATION_SEPARATOR_WIDTH_71 = 71;   // Validation results separator
    const int VALIDATION_SEPARATOR_WIDTH_72 = 72;   // Validation section separator
    const int CHART_HEADER_WIDTH_78 = 78;           // Chart header separator
    const int ENHANCED_CHART_SEPARATOR_WIDTH_95 = 95; // Enhanced chart separator width
    
    // Number formatting constants
    const int NUMBER_FORMAT_PRECISION = 5;          // Default precision for number formatting
    const int TEMPERATURE_DISPLAY_PRECISION = 1;    // Precision for temperature display
    const int PERIOD_LABEL_MAX_LENGTH = 4;          // Maximum length for period labels
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