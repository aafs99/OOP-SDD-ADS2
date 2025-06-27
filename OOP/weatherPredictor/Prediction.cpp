#include "Prediction.h"
#include "PredictionDisplay.h"
#include "Common.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>

// Core prediction algorithms
//self written code start
PredictionResult Prediction::predictLinearWithConfidence(const std::vector<Candlestick>& data) {
    const std::string modelName = "Linear Regression";
    std::string errorMessage;
    
    if (!validateDataSize(data, Constants::MIN_LINEAR_DATA_SIZE, errorMessage)) {
        return PredictionResult(errorMessage, modelName);
    }
    
    const auto n = static_cast<double>(data.size());
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    
    // Calculate sums for least squares method
    for (size_t i = 0; i < data.size(); ++i) {
        const double x = static_cast<double>(i);
        const double y = data[i].getClose();
        
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }
    
    // Calculate slope and intercept
    const double denominator = n * sumX2 - sumX * sumX;
    if (std::abs(denominator) < Constants::EPSILON) {
        const double prediction = sumY / n;
        const std::string confDesc = "R² = 0.0 (no linear trend detected)";
        return PredictionResult(prediction, 0.0, modelName, confDesc);
    }
    
    const double slope = (n * sumXY - sumX * sumY) / denominator;
    const double intercept = (sumY - slope * sumX) / n;
    
    // Calculate prediction for next period
    const double nextX = n;
    const double prediction = slope * nextX + intercept;
    
    // Calculate confidence metric (R²)
    const double rSquared = calculateRSquaredDetailed(data, slope, intercept);
    
    std::ostringstream confDesc;
    confDesc << "R² = " << std::fixed << std::setprecision(3) << rSquared 
             << " (coefficient of determination)";
    
    return PredictionResult(prediction, rSquared, modelName, confDesc.str());
}
// self written code end

//self written code start
PredictionResult Prediction::predictMovingAverageWithConfidence(
    const std::vector<Candlestick>& data, int windowSize) {
    
    std::ostringstream modelNameStream;
    modelNameStream << "Moving Average (" << windowSize << "-period)";
    const std::string modelName = modelNameStream.str();
    std::string errorMessage;
    
    if (!validateDataSize(data, Constants::MIN_MOVING_AVERAGE_DATA_SIZE, errorMessage)) {
        return PredictionResult(errorMessage, modelName);
    }
    
    if (!validateWindowSize(windowSize, data.size(), errorMessage)) {
        return PredictionResult(errorMessage, modelName);
    }
    
    const int actualWindowSize = std::min(windowSize, static_cast<int>(data.size()));
    const auto startIdx = data.size() - actualWindowSize;
    
    // Calculate moving average prediction
    const double sum = std::accumulate(
        data.begin() + startIdx, data.end(), 0.0,
        [](double acc, const Candlestick& candle) {
            return acc + candle.getClose();
        });
    
    const double prediction = sum / actualWindowSize;
    
    // Calculate stability confidence metric
    const double stabilityConfidence = calculateStabilityConfidence(data, actualWindowSize);
    
    std::ostringstream confDesc;
    confDesc << "Stability = " << std::fixed << std::setprecision(3) << stabilityConfidence
             << " (inverse volatility metric)";
    
    return PredictionResult(prediction, stabilityConfidence, modelName, confDesc.str());
}

PredictionResult Prediction::predictHeuristicWithConfidence(const std::vector<Candlestick>& data) {
    const std::string modelName = "Heuristic (Momentum)";
    std::string errorMessage;
    
    if (!validateDataSize(data, Constants::MIN_HEURISTIC_DATA_SIZE, errorMessage)) {
        return PredictionResult(errorMessage, modelName);
    }
    
    if (data.size() == 1) {
        const std::string confDesc = "Single data point (no momentum available)";
        return PredictionResult(data[0].getClose(), 0.0, modelName, confDesc);
    }
    
    // Calculate momentum prediction
    const double last = data.back().getClose();
    const double secondLast = data[data.size() - 2].getClose();
    const double change = last - secondLast;
    const double prediction = last + change;
    
    // Calculate consistency confidence metric
    const double consistencyConfidence = calculateConsistencyConfidence(data);
    
    std::ostringstream confDesc;
    confDesc << "Consistency = " << std::fixed << std::setprecision(3) << consistencyConfidence
             << " (trend reliability metric)";
    
    return PredictionResult(prediction, consistencyConfidence, modelName, confDesc.str());
}

void Prediction::displayPredictionResults(
    const std::vector<Candlestick>& data,
    TimeFrame timeframe,
    std::string_view country,
    int startYear, int endYear,
    const PredictionConfig& config) {
    
    PredictionDisplay::displayEnhancedResults(data, timeframe, country, startYear, endYear, config);
}
// self written code end

// Private helper methods

double Prediction::calculateRSquaredDetailed(
    const std::vector<Candlestick>& data, 
    double slope, double intercept) noexcept {
    
    if (data.size() < Constants::MIN_LINEAR_DATA_SIZE) return 0.0;
    
    // Calculate mean efficiently
    const double sumY = std::accumulate(data.begin(), data.end(), 0.0,
        [](double acc, const Candlestick& candle) {
            return acc + candle.getClose();
        });
    const double meanY = sumY / data.size();
    
    double totalSumSquares = 0.0;
    double residualSumSquares = 0.0;
    
    for (size_t i = 0; i < data.size(); ++i) {
        const double actualY = data[i].getClose();
        const double predictedY = slope * static_cast<double>(i) + intercept;
        
        totalSumSquares += std::pow(actualY - meanY, 2);
        residualSumSquares += std::pow(actualY - predictedY, 2);
    }
    
    if (totalSumSquares < Constants::EPSILON) return 0.0;
    
    const double rSquared = 1.0 - (residualSumSquares / totalSumSquares);
    return std::max(0.0, rSquared);
}

double Prediction::calculateStabilityConfidence(
    const std::vector<Candlestick>& data, 
    int windowSize) noexcept {
    
    if (data.size() < Constants::MIN_MOVING_AVERAGE_DATA_SIZE || windowSize < 2) return 0.0;
    
    const auto start = std::max(0, static_cast<int>(data.size()) - windowSize);
    std::vector<double> recentValues;
    recentValues.reserve(windowSize);
    
    for (int i = start; i < static_cast<int>(data.size()); ++i) {
        recentValues.push_back(data[i].getClose());
    }
    
    if (recentValues.size() < Constants::MIN_MOVING_AVERAGE_DATA_SIZE) return 0.0;
    
    const double mean = calculateMean(recentValues);
    const double stdDev = calculateStandardDeviation(recentValues, mean);
    
    const double relativeVolatility = (mean > 0) ? stdDev / std::abs(mean) : stdDev;
    const double stabilityConfidence = 1.0 / (1.0 + relativeVolatility * Constants::STABILITY_SCALE_FACTOR);
    
    return std::max(0.0, std::min(stabilityConfidence, 1.0));
}

double Prediction::calculateConsistencyConfidence(const std::vector<Candlestick>& data) noexcept {
    if (data.size() < 3) return 0.0;
    
    std::vector<double> changes;
    changes.reserve(data.size() - 1);
    
    for (size_t i = 1; i < data.size(); ++i) {
        changes.push_back(data[i].getClose() - data[i-1].getClose());
    }
    
    if (changes.size() < Constants::MIN_MOVING_AVERAGE_DATA_SIZE) return 0.0;
    
    const double changeMean = calculateMean(changes);
    const double changeStdDev = calculateStandardDeviation(changes, changeMean);
    
    const double changeVariability = changeStdDev / (std::abs(changeMean) + 1.0);
    const double consistencyConfidence = 1.0 / (1.0 + changeVariability);
    
    return std::max(0.0, std::min(consistencyConfidence, 1.0));
}
// self written code end

bool Prediction::validateDataSize(
    const std::vector<Candlestick>& data, 
    size_t minSize, 
    std::string& errorMessage) noexcept {
    
    if (data.size() < minSize) {
        std::ostringstream oss;
        oss << "Insufficient data (need at least " << minSize << " points, got " << data.size() << ")";
        errorMessage = oss.str();
        return false;
    }
    return true;
}

bool Prediction::validateWindowSize(
    int windowSize, 
    size_t dataSize, 
    std::string& errorMessage) noexcept {
    
    if (windowSize <= 0) {
        errorMessage = "Window size must be positive";
        return false;
    }
    
    if (static_cast<size_t>(windowSize) > dataSize) {
        std::ostringstream oss;
        oss << "Window size (" << windowSize << ") cannot exceed data size (" << dataSize << ")";
        errorMessage = oss.str();
        return false;
    }
    
    return true;
}

std::string Prediction::getConfidenceLevel(double confidence) noexcept {
    if (confidence > Constants::HIGH_CONFIDENCE_THRESHOLD) {
        return "High";
    } else if (confidence > Constants::MODERATE_CONFIDENCE_THRESHOLD) {
        return "Moderate";
    } else {
        return "Low";
    }
}

//self written code start
std::string Prediction::determineNextPeriod(
    const std::vector<Candlestick>& data, 
    TimeFrame timeframe, int endYear) {
    
    if (data.empty()) return "Unknown";
    
    switch (timeframe) {
        case TimeFrame::Yearly:
            return std::to_string(endYear + 1);
            
        case TimeFrame::Monthly: {
            const std::string& lastDate = data.back().getDate();
            if (lastDate.length() >= 7) {
                const int year = std::stoi(lastDate.substr(0, 4));
                const int month = std::stoi(lastDate.substr(5, 2));
                
                if (month == 12) {
                    return std::to_string(year + 1) + "-01 (January " + std::to_string(year + 1) + ")";
                } else {
                    std::ostringstream oss;
                    oss << year << "-" << std::setfill('0') << std::setw(2) << (month + 1);
                    return oss.str();
                }
            }
            return "Next month";
        }
            
        case TimeFrame::Daily:
            return "Next day";
            
        default:
            return "Next period";
    }
}
// self written code end