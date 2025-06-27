#ifndef PREDICTION_H
#define PREDICTION_H

#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <sstream>
#include <iomanip>
#include <limits>
#include "Candlestick.h"
#include "Common.h"

/**
 * Structure to hold prediction results with confidence metrics
 */
struct PredictionResult {
    double predictionValue{0.0};
    double confidenceMetric{0.0};
    std::string modelName;
    bool isValid{false};
    std::string errorMessage;
    std::string confidenceDescription;
    
    // Constructor for successful prediction
    PredictionResult(double prediction, double confidence, std::string_view name, 
                    std::string_view confDesc = "") 
        : predictionValue(prediction), confidenceMetric(confidence), modelName(name),
          isValid(true), confidenceDescription(confDesc) {}
    
    // Constructor for failed prediction
    PredictionResult(std::string_view error, std::string_view name)
        : modelName(name), isValid(false), errorMessage(error) {}
    
    // Move semantics support
    PredictionResult(PredictionResult&&) = default;
    PredictionResult& operator=(PredictionResult&&) = default;
    PredictionResult(const PredictionResult&) = default;
    PredictionResult& operator=(const PredictionResult&) = default;
};

/**
 * Enhanced structure to hold cross-validation results
 */
struct ValidationResult {
    double meanAbsoluteError{0.0};
    double meanSquaredError{0.0}; 
    double maxError{0.0};
    double minError{0.0};
    int validPredictions{0};
    int totalAttempts{0};
    bool isValid{false};
    std::string errorMessage;
    
    // Calculated properties
    [[nodiscard]] double getRMSE() const noexcept { 
        return std::sqrt(meanSquaredError); 
    }
    
    [[nodiscard]] double getSuccessRate() const noexcept {
        return totalAttempts > 0 ? 
               static_cast<double>(validPredictions) / totalAttempts * 100.0 : 0.0;
    }
};

/**
 * Function pointer type for prediction models
 */
using PredictionFunction = std::function<PredictionResult(const std::vector<Candlestick>&)>;

/**
 * Configuration for prediction display
 */
struct PredictionConfig {
    bool showValidation{false};
    bool showChart{false};
    int chartWidth{50};
    int movingAverageWindow{3};
    
    static constexpr double HIGH_CONFIDENCE_THRESHOLD = 0.7;
    static constexpr double MODERATE_CONFIDENCE_THRESHOLD = 0.4;
    static constexpr double STABILITY_SCALE_FACTOR = 5.0;
    static constexpr double CHART_PADDING_RATIO = 0.1;
};

/**
 * Provides robust prediction methods with confidence metrics and cross-validation
 * Modern C++ implementation with improved error handling and performance
 */
class Prediction {
public:
    // Enhanced prediction methods
    [[nodiscard]] static PredictionResult predictLinearWithConfidence(
        const std::vector<Candlestick>& data);
    
    [[nodiscard]] static PredictionResult predictMovingAverageWithConfidence(
        const std::vector<Candlestick>& data, int windowSize = 3);
    
    [[nodiscard]] static PredictionResult predictHeuristicWithConfidence(
        const std::vector<Candlestick>& data);

    // Cross-validation methods
    [[nodiscard]] static ValidationResult validateModel(
        const std::vector<Candlestick>& data,
        const PredictionFunction& modelFunction,
        int minTrainingSize = 2);

    [[nodiscard]] static std::vector<ValidationResult> validateAllModels(
        const std::vector<Candlestick>& data);

    // Rolling predictions for visualization
    [[nodiscard]] static std::vector<PredictionResult> generateRollingPredictions(
        const std::vector<Candlestick>& data,
        const PredictionFunction& modelFunction,
        int startIndex = 2);

    // Display methods
    static void displayEnhancedPredictionResults(
        const std::vector<Candlestick>& data,
        TimeFrame timeframe,
        std::string_view country,
        int startYear, int endYear,
        const PredictionConfig& config = {});

    static void displayPredictionComparisonChart(
        const std::vector<Candlestick>& data,
        TimeFrame timeframe,
        std::string_view country,
        int startYear, int endYear,
        const PredictionConfig& config = {});

    static void displayValidationResults(const std::vector<ValidationResult>& results);
    
    // New helper display methods
    static void displayPredictionSummary(
        const std::vector<PredictionResult>& results,
        const std::string& nextPeriod);
    
    static void displayPredictionTable(
        const std::vector<PredictionResult>& results,
        const std::string& nextPeriod);
    
    static void displayRecommendation(const std::vector<PredictionResult>& results);
    
    static void displayChartHeader();
    static void displayChartLegend();
    
    static void displayChartData(
        const std::vector<Candlestick>& data,
        const std::vector<std::vector<PredictionResult>>& allPredictions,
        TimeFrame timeframe,
        int startYear,
        const PredictionConfig& config);
    
    static void displayChartFooter(
        const std::vector<Candlestick>& data,
        const PredictionConfig& config);
    
    static void displayAccuracyAnalysis(
        const std::vector<Candlestick>& data,
        const std::vector<std::vector<PredictionResult>>& allPredictions);
    
    static std::vector<std::vector<PredictionResult>> generateAllRollingPredictions(
        const std::vector<Candlestick>& data);

    // Legacy compatibility methods
    [[nodiscard]] static double predictLinear(const std::vector<Candlestick>& data);
    [[nodiscard]] static double predictMovingAverage(const std::vector<Candlestick>& data, int windowSize = 3);
    [[nodiscard]] static double predictHeuristic(const std::vector<Candlestick>& data);
    
    static void displayPredictionResults(
        const std::vector<Candlestick>& data,
        TimeFrame timeframe,
        std::string_view country,
        int startYear, int endYear);

    // Utility methods
    [[nodiscard]] static double calculateLinearRSquared(const std::vector<Candlestick>& data);
    [[nodiscard]] static std::string determineNextPeriod(
        const std::vector<Candlestick>& data,
        TimeFrame timeframe, int endYear);

private:
    // Statistical calculation methods
    [[nodiscard]] static double calculateRSquaredDetailed(
        const std::vector<Candlestick>& data, 
        double slope, double intercept) noexcept;

    [[nodiscard]] static double calculateStabilityConfidence(
        const std::vector<Candlestick>& data, 
        int windowSize) noexcept;

    [[nodiscard]] static double calculateConsistencyConfidence(
        const std::vector<Candlestick>& data) noexcept;

    // Statistical utilities
    template<typename Container>
    [[nodiscard]] static double calculateMean(const Container& values) noexcept;

    template<typename Container>
    [[nodiscard]] static double calculateStandardDeviation(
        const Container& values, double mean) noexcept;

    // Formatting utility methods
    [[nodiscard]] static std::string formatTemperature(double temp, int precision = 1);
    [[nodiscard]] static std::string formatConfidence(double confidence, int precision = 3);
    [[nodiscard]] static std::string getConfidenceLevel(double confidence) noexcept;
    [[nodiscard]] static std::string getMonthName(int month) noexcept;
    
    // String formatting helpers
    [[nodiscard]] static std::string formatDouble(double value, int precision = 2);
    
    // Chart utilities
    [[nodiscard]] static int calculateChartPosition(
        double value, double minVal, double maxVal, int chartWidth) noexcept;
    
    static void generateChartLine(
        std::vector<char>& chartLine,
        const std::vector<std::pair<double, char>>& values,
        double minTemp, double maxTemp) noexcept;

    // Input validation
    [[nodiscard]] static bool validateDataSize(
        const std::vector<Candlestick>& data, 
        size_t minSize, 
        std::string& errorMessage) noexcept;

    [[nodiscard]] static bool validateWindowSize(
        int windowSize, 
        size_t dataSize, 
        std::string& errorMessage) noexcept;

    // Constants
    static constexpr double EPSILON = 1e-10;
    static constexpr size_t MIN_LINEAR_DATA_SIZE = 2;
    static constexpr size_t MIN_HEURISTIC_DATA_SIZE = 2;
    static constexpr int DEFAULT_CHART_WIDTH = 50;
    static constexpr int MAX_CHART_WIDTH = 120;
};

// Template implementations
template<typename Container>
double Prediction::calculateMean(const Container& values) noexcept {
    if (values.empty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& value : values) {
        sum += value;
    }
    return sum / values.size();
}

template<typename Container>
double Prediction::calculateStandardDeviation(const Container& values, double mean) noexcept {
    if (values.size() < 2) return 0.0;
    
    double sumSquaredDiffs = 0.0;
    for (const auto& value : values) {
        const double diff = value - mean;
        sumSquaredDiffs += diff * diff;
    }
    
    return std::sqrt(sumSquaredDiffs / (values.size() - 1));
}

#endif // PREDICTION_H