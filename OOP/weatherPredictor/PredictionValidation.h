#ifndef PREDICTION_VALIDATION_H
#define PREDICTION_VALIDATION_H

#include <vector>
#include <string>
#include <functional>
#include "Prediction.h"
#include "Candlestick.h"

//self written code start
/**
 * Hold cross-validation results
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
 * Handles cross-validation of prediction models
 */
class PredictionValidation {
public:
    // Validation methods
    static ValidationResult validateModel(
        const std::vector<Candlestick>& data,
        const PredictionFunction& modelFunction,
        int minTrainingSize = 2
    );

    static std::vector<ValidationResult> validateAllModels(const std::vector<Candlestick>& data);

    // Display methods
    static void displayValidationResults(const std::vector<Candlestick>& data);

private:
    // Helper methods
    template<typename Container>
    static double calculateMean(const Container& values) noexcept;
    
    static std::string formatDouble(double value, int precision = 2);
};

// Template implementation
template<typename Container>
double PredictionValidation::calculateMean(const Container& values) noexcept {
    if (values.empty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& value : values) {
        sum += value;
    }
    return sum / values.size();
}

#endif // PREDICTION_VALIDATION_H
//self written code end