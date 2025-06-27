#include "PredictionDisplay.h"
#include "PredictionValidation.h"
#include "PredictionChart.h"
#include "Prediction.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>

//self written code start
void PredictionDisplay::displayEnhancedResults(
    const std::vector<Candlestick>& data,
    TimeFrame timeframe,
    std::string_view country,
    int startYear, int endYear,
    const PredictionConfig& config) {
    
    if (data.empty()) {
        std::cout << "Cannot generate predictions: No data available.\n";
        return;
    }

    const std::string nextPeriod = Prediction::determineNextPeriod(data, timeframe, endYear);
    
    // Display header
    displayHeader(country, startYear, endYear, timeframe, data.size(), nextPeriod);

    // Generate predictions
    const auto results = std::vector<PredictionResult>{
        Prediction::predictLinearWithConfidence(data),
        Prediction::predictMovingAverageWithConfidence(data, config.movingAverageWindow),
        Prediction::predictHeuristicWithConfidence(data)
    };

    // Display prediction summary
    displayPredictionSummary(results, nextPeriod);

    // Show validation if requested and sufficient data
    if (config.showValidation && data.size() >= 4) {
        std::cout << "\n" << std::string(Constants::VALIDATION_SEPARATOR_WIDTH_72, '-') << "\n";
        std::cout << "CROSS-VALIDATION ANALYSIS\n";
        std::cout << std::string(Constants::VALIDATION_SEPARATOR_WIDTH_72, '-') << "\n";
        PredictionValidation::displayValidationResults(data);
    }

    // Show chart if requested and sufficient data
    if (config.showChart && data.size() >= 4) {
        PredictionChart::displayVerticalChart(data, timeframe, country, startYear, endYear);
    }
    
    std::cout << "\n";
}

void PredictionDisplay::displayHeader(std::string_view country, int startYear, int endYear, 
                                     TimeFrame timeframe, size_t dataSize, const std::string& nextPeriod) {
    std::cout << "\n" << std::string(Constants::CHART_HEADER_WIDTH_78, '=') << "\n";
    std::cout << "TEMPERATURE PREDICTION ANALYSIS\n";
    std::cout << std::string(Constants::CHART_HEADER_WIDTH_78, '=') << "\n";
    std::cout << "Country: " << country << "\n";
    std::cout << "Data Range: " << startYear << "-" << endYear 
              << " (" << Utils::timeFrameToString(timeframe) << " data)\n";
    std::cout << "Data Points: " << dataSize << " periods\n";
    std::cout << "Predicting: " << nextPeriod << "\n\n";
}

void PredictionDisplay::displayPredictionSummary(
    const std::vector<PredictionResult>& results,
    const std::string& nextPeriod) {
    
    std::cout << "PREDICTION METHODS WITH CONFIDENCE METRICS\n\n";

    const std::vector<std::string> descriptions = {
        "Least squares trend line fitting - identifies long-term temperature trends",
        "Average of recent temperature values - smooths short-term fluctuations", 
        "Projects recent temperature change forward - assumes momentum continues"
    };

    for (size_t i = 0; i < results.size() && i < descriptions.size(); ++i) {
        const auto& result = results[i];
        
        std::cout << (i + 1) << ". " << result.modelName << "\n";
        std::cout << "   Method: " << descriptions[i] << "\n";
        
        if (result.isValid) {
            std::cout << "   Result: " << formatTemperature(result.predictionValue) << "°C\n";
            std::cout << "   Confidence: " << result.confidenceDescription << "\n";
            std::cout << "   Assessment: " << Prediction::getConfidenceLevel(result.confidenceMetric) << " confidence\n\n";
        } else {
            std::cout << "   Error: " << result.errorMessage << "\n\n";
        }
    }

    // Display prediction table and recommendation
    displayPredictionTable(results, nextPeriod);
    displayRecommendation(results);
}

void PredictionDisplay::displayPredictionTable(
    const std::vector<PredictionResult>& results,
    const std::string& nextPeriod) {
    
    std::cout << "PREDICTION SUMMARY FOR " << nextPeriod << "\n";
    std::cout << std::string(Constants::ENHANCED_CHART_SEPARATOR_WIDTH_95, '-') << "\n";
    std::cout << std::left << std::setw(25) << "Method" 
              << std::setw(12) << "Prediction" 
              << std::setw(15) << "Confidence"
              << std::setw(20) << "Reliability" << "\n";
    std::cout << std::string(Constants::ENHANCED_CHART_SEPARATOR_WIDTH_95, '-') << "\n";
    
    for (const auto& result : results) {
        if (result.isValid) {
            std::cout << std::left << std::setw(25) << result.modelName
                      << std::setw(12) << (formatTemperature(result.predictionValue) + "°C")
                      << std::setw(15) << formatConfidence(result.confidenceMetric)
                      << std::setw(20) << Prediction::getConfidenceLevel(result.confidenceMetric) << "\n";
        } else {
            std::cout << std::left << std::setw(25) << result.modelName 
                      << std::setw(50) << ("Error: " + result.errorMessage) << "\n";
        }
    }
    std::cout << "\n";
}

void PredictionDisplay::displayRecommendation(const std::vector<PredictionResult>& results) {
    const auto bestResult = std::max_element(results.begin(), results.end(),
        [](const PredictionResult& a, const PredictionResult& b) {
            if (!a.isValid) return true;
            if (!b.isValid) return false;
            return a.confidenceMetric < b.confidenceMetric;
        });

    std::cout << "CONFIDENCE-BASED RECOMMENDATION\n";
    std::cout << std::string(Constants::SECTION_SEPARATOR_WIDTH_60, '-') << "\n";
    
    if (bestResult != results.end() && bestResult->isValid && bestResult->confidenceMetric > 0.0) {
        std::cout << "   RECOMMENDED: " << bestResult->modelName << "\n";
        std::cout << "   Confidence: " << formatConfidence(bestResult->confidenceMetric) << "\n";
        std::cout << "   Reason: Highest confidence metric among available models\n";
    } else {
        std::cout << "⚠ No reliable model available - consider gathering more data\n";
    }
    std::cout << "\n";
}

// Formatting utilities

std::string PredictionDisplay::formatTemperature(double temp, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << temp;
    return oss.str();
}

std::string PredictionDisplay::formatConfidence(double confidence, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << confidence;
    return oss.str();
}

std::string PredictionDisplay::formatDouble(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}
// self written code end