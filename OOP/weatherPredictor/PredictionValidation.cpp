#include "PredictionValidation.h"
#include "Prediction.h"
#include "Common.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <sstream>

ValidationResult PredictionValidation::validateModel(
    const std::vector<Candlestick>& data,
    const PredictionFunction& modelFunction,
    int minTrainingSize) {
    
    ValidationResult result;
    
    if (static_cast<int>(data.size()) < minTrainingSize + 1) {
        std::ostringstream errorStream;
        errorStream << "Insufficient data for validation (need at least " 
                   << (minTrainingSize + 1) << " points)";
        result.errorMessage = errorStream.str();
        return result;
    }
    
    std::vector<double> errors, squaredErrors;
    const size_t reserveSize = data.size() - static_cast<size_t>(minTrainingSize);
    errors.reserve(reserveSize);
    squaredErrors.reserve(reserveSize);
    
    // Perform leave-one-out cross-validation
    for (size_t testIndex = static_cast<size_t>(minTrainingSize); testIndex < data.size(); ++testIndex) {
        // Create training data efficiently
        const std::vector<Candlestick> trainingData(data.begin(), data.begin() + static_cast<long>(testIndex));
        
        try {
            const PredictionResult predResult = modelFunction(trainingData);
            
            if (predResult.isValid) {
                const double actualValue = data[testIndex].getClose();
                const double error = std::abs(predResult.predictionValue - actualValue);
                const double squaredError = std::pow(predResult.predictionValue - actualValue, 2);
                
                errors.push_back(error);
                squaredErrors.push_back(squaredError);
                ++result.validPredictions;
            }
        } catch (const std::exception&) {
            // Handle any exceptions from prediction function
            // Continue with next iteration
        }
        
        ++result.totalAttempts;
    }
    
    if (errors.empty()) {
        result.errorMessage = "No valid predictions generated during validation";
        return result;
    }
    
    // Calculate statistical metrics efficiently
    result.meanAbsoluteError = calculateMean(errors);
    result.meanSquaredError = calculateMean(squaredErrors);
    result.maxError = *std::max_element(errors.begin(), errors.end());
    result.minError = *std::min_element(errors.begin(), errors.end());
    result.isValid = true;
    
    return result;
}

std::vector<ValidationResult> PredictionValidation::validateAllModels(const std::vector<Candlestick>& data) {
    std::vector<ValidationResult> results;
    results.reserve(3);
    
    // Define prediction functions using lambdas
    const std::vector<std::pair<PredictionFunction, int>> models = {
        {[](const std::vector<Candlestick>& d) { return Prediction::predictLinearWithConfidence(d); }, 
         static_cast<int>(Constants::MIN_LINEAR_DATA_SIZE)},
        {[](const std::vector<Candlestick>& d) { return Prediction::predictMovingAverageWithConfidence(d, 3); }, 
         static_cast<int>(Constants::MIN_MOVING_AVERAGE_DATA_SIZE)},
        {[](const std::vector<Candlestick>& d) { return Prediction::predictHeuristicWithConfidence(d); }, 
         static_cast<int>(Constants::MIN_HEURISTIC_DATA_SIZE)}
    };
    
    for (const auto& [modelFunc, minSize] : models) {
        results.push_back(validateModel(data, modelFunc, minSize));
    }
    
    return results;
}

void PredictionValidation::displayValidationResults(const std::vector<Candlestick>& data) {
    const auto results = validateAllModels(data);
    
    if (results.empty()) {
        std::cout << "No validation results to display.\n";
        return;
    }

    const std::vector<std::string> modelNames = {"Linear Regression", "Moving Average (3)", "Heuristic (Momentum)"};
    
    std::cout << "Cross-Validation Results:\n";
    std::cout << std::left << std::setw(20) << "Model" 
              << std::setw(12) << "MAE (°C)" 
              << std::setw(12) << "RMSE (°C)"
              << std::setw(12) << "Max Error"
              << std::setw(15) << "Success Rate" << "\n";
    std::cout << std::string(Constants::VALIDATION_SEPARATOR_WIDTH_71, '-') << "\n";
    
    for (size_t i = 0; i < results.size() && i < modelNames.size(); i++) {
        const ValidationResult& result = results[i];
        
        if (result.isValid) {
            double rmse = result.getRMSE();
            double successRate = result.getSuccessRate();
            
            std::cout << std::left << std::setw(20) << modelNames[i]
                      << std::setw(12) << formatDouble(result.meanAbsoluteError, Constants::TEMPERATURE_DISPLAY_PRECISION + 1)
                      << std::setw(12) << formatDouble(rmse, Constants::TEMPERATURE_DISPLAY_PRECISION + 1)
                      << std::setw(12) << formatDouble(result.maxError, Constants::TEMPERATURE_DISPLAY_PRECISION + 1)
                      << std::setw(15) << (formatDouble(successRate, Constants::TEMPERATURE_DISPLAY_PRECISION) + "%") << "\n";
        } else {
            std::cout << std::left << std::setw(20) << modelNames[i] 
                      << std::setw(50) << ("Error: " + result.errorMessage) << "\n";
        }
    }
    
    // Find best performing model
    double bestMAE = std::numeric_limits<double>::max();
    std::string bestValidationModel = "None";
    
    for (size_t i = 0; i < results.size() && i < modelNames.size(); i++) {
        if (results[i].isValid && results[i].meanAbsoluteError < bestMAE) {
            bestMAE = results[i].meanAbsoluteError;
            bestValidationModel = modelNames[i];
        }
    }
    
    if (bestValidationModel != "None") {
        std::cout << "\n VALIDATION PERFORMANCE: " << bestValidationModel 
                  << " (MAE: " << formatDouble(bestMAE, Constants::TEMPERATURE_DISPLAY_PRECISION + 1) << "°C)\n";
        std::cout << "   Note: Lower MAE indicates better historical prediction accuracy\n";
    }
}

std::string PredictionValidation::formatDouble(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}