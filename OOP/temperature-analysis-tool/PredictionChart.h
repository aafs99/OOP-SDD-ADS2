#ifndef PREDICTION_CHART_H
#define PREDICTION_CHART_H

#include <vector>
#include <string>
#include <string_view>
#include "Prediction.h"
#include "Candlestick.h"
#include "Common.h"

//self written code start
/**
 * Handles vertical chart display for prediction visualization
 */
class PredictionChart {
public:
    // Main chart display method
    static void displayVerticalChart(
        const std::vector<Candlestick>& data,
        TimeFrame timeframe,
        std::string_view country,
        int startYear, int endYear
    );

private:
    // Chart generation methods
    static std::vector<std::vector<PredictionResult>> generateAllRollingPredictions(
        const std::vector<Candlestick>& data
    );
    
    static std::vector<PredictionResult> generateRollingPredictions(
        const std::vector<Candlestick>& data,
        const std::function<PredictionResult(const std::vector<Candlestick>&)>& modelFunction,
        int startIndex
    );

    // Chart display helpers
    static void displayChartHeader(const std::vector<Candlestick>& data, std::string_view country, 
                                  int startYear, int endYear);
    static void displayChart(const std::vector<Candlestick>& data, 
                           const std::vector<std::vector<PredictionResult>>& allPredictions,
                           TimeFrame timeframe, int startYear);
    static void displayChartFooter();
    static void displayAccuracyAnalysis(const std::vector<Candlestick>& data,
                                       const std::vector<std::vector<PredictionResult>>& allPredictions);

    // Chart utility methods
    static char getSymbolAtPosition(const std::vector<Candlestick>& data,
                                   const std::vector<std::vector<PredictionResult>>& allPredictions,
                                   size_t periodIndex, double currentTemp, double tempPerRow);
    static void calculateTemperatureRange(const std::vector<Candlestick>& data,
                                         const std::vector<std::vector<PredictionResult>>& allPredictions,
                                         double& minTemp, double& maxTemp);
    static std::string formatPeriodLabel(size_t index, TimeFrame timeframe, int startYear);
    
    // Statistical utilities
    template<typename Container>
    static double calculateMean(const Container& values) noexcept;
    
    static std::string formatDouble(double value, int precision = 2);
};

// Template implementation
template<typename Container>
double PredictionChart::calculateMean(const Container& values) noexcept {
    if (values.empty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& value : values) {
        sum += value;
    }
    return sum / values.size();
}

#endif // PREDICTION_CHART_H

//self written code start