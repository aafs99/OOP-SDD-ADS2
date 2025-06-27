#ifndef PREDICTION_H
#define PREDICTION_H

#include <vector>
#include <string>
#include <functional>
#include <string_view>
#include "Candlestick.h"
#include "Common.h"

//self written code start
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
    
    // Default constructors
    PredictionResult() = default;
    PredictionResult(const PredictionResult&) = default;
    PredictionResult& operator=(const PredictionResult&) = default;
    PredictionResult(PredictionResult&&) = default;
    PredictionResult& operator=(PredictionResult&&) = default;
};

/**
 * Configuration for prediction display
 */
struct PredictionConfig {
    bool showValidation{true};
    bool showChart{true};
    int movingAverageWindow{3};
};

/**
 * Core prediction algorithms
 */
class Prediction {
public:
    // Main prediction methods
    static PredictionResult predictLinearWithConfidence(const std::vector<Candlestick>& data);
    static PredictionResult predictMovingAverageWithConfidence(const std::vector<Candlestick>& data, int windowSize = 3);
    static PredictionResult predictHeuristicWithConfidence(const std::vector<Candlestick>& data);

    // Main display method
    static void displayPredictionResults(
        const std::vector<Candlestick>& data,
        TimeFrame timeframe,
        std::string_view country,
        int startYear, int endYear,
        const PredictionConfig& config = {}
    );
    // Utility methods
    static std::string getConfidenceLevel(double confidence) noexcept;
    static std::string determineNextPeriod(const std::vector<Candlestick>& data, TimeFrame timeframe, int endYear);

private:
    // Statistical calculation methods
    static double calculateRSquaredDetailed(const std::vector<Candlestick>& data, double slope, double intercept) noexcept;
    static double calculateStabilityConfidence(const std::vector<Candlestick>& data, int windowSize) noexcept;
    static double calculateConsistencyConfidence(const std::vector<Candlestick>& data) noexcept;

    // Statistical utilities
    template<typename Container>
    static double calculateMean(const Container& values) noexcept;

    template<typename Container>
    static double calculateStandardDeviation(const Container& values, double mean) noexcept;

    // Input validation
    static bool validateDataSize(const std::vector<Candlestick>& data, size_t minSize, std::string& errorMessage) noexcept;
    static bool validateWindowSize(int windowSize, size_t dataSize, std::string& errorMessage) noexcept;
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
//self written code end