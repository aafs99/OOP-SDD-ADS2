#ifndef PREDICTION_DISPLAY_H
#define PREDICTION_DISPLAY_H

#include <vector>
#include <string>
#include <string_view>
#include "Prediction.h"
#include "Candlestick.h"
#include "Common.h"

//self written code start
/**
 * Handles all prediction display and formatting functionality
 */
class PredictionDisplay {
public:
    // Main display method
    static void displayEnhancedResults(
        const std::vector<Candlestick>& data,
        TimeFrame timeframe,
        std::string_view country,
        int startYear, int endYear,
        const PredictionConfig& config
    );

private:
    // Display components
    static void displayHeader(std::string_view country, int startYear, int endYear, 
                             TimeFrame timeframe, size_t dataSize, const std::string& nextPeriod);
    static void displayPredictionSummary(const std::vector<PredictionResult>& results, const std::string& nextPeriod);
    static void displayPredictionTable(const std::vector<PredictionResult>& results, const std::string& nextPeriod);
    static void displayRecommendation(const std::vector<PredictionResult>& results);

    // Formatting utilities
    static std::string formatTemperature(double temp, int precision = 1);
    static std::string formatConfidence(double confidence, int precision = 3);
    static std::string formatDouble(double value, int precision = 2);
};

#endif // PREDICTION_DISPLAY_H
//self written code start