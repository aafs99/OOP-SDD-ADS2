#include "Prediction.h"
#include "Common.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <sstream>

// Enhanced prediction methods with improved error handling and performance

PredictionResult Prediction::predictLinearWithConfidence(const std::vector<Candlestick>& data) {
    const std::string modelName = "Linear Regression";
    std::string errorMessage;
    
    if (!validateDataSize(data, MIN_LINEAR_DATA_SIZE, errorMessage)) {
        return PredictionResult(errorMessage, modelName);
    }
    
    const auto n = static_cast<double>(data.size());
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    
    // Calculate sums using range-based for loop with index
    for (size_t i = 0; i < data.size(); ++i) {
        const double x = static_cast<double>(i);
        const double y = data[i].getClose();
        
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }
    
    // Calculate slope and intercept using least squares method
    const double denominator = n * sumX2 - sumX * sumX;
    if (std::abs(denominator) < EPSILON) {
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

PredictionResult Prediction::predictMovingAverageWithConfidence(
    const std::vector<Candlestick>& data, int windowSize) {
    
    std::ostringstream modelNameStream;
    modelNameStream << "Moving Average (" << windowSize << "-period)";
    const std::string modelName = modelNameStream.str();
    std::string errorMessage;
    
    if (!validateDataSize(data, 1, errorMessage)) {
        return PredictionResult(errorMessage, modelName);
    }
    
    if (!validateWindowSize(windowSize, data.size(), errorMessage)) {
        return PredictionResult(errorMessage, modelName);
    }
    
    const int actualWindowSize = std::min(windowSize, static_cast<int>(data.size()));
    const auto startIdx = data.size() - actualWindowSize;
    
    // Calculate moving average prediction using more efficient approach
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
    
    if (!validateDataSize(data, 1, errorMessage)) {
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

// Cross-validation implementation with improved error handling

ValidationResult Prediction::validateModel(
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
    errors.reserve(data.size() - minTrainingSize);
    squaredErrors.reserve(data.size() - minTrainingSize);
    
    // Perform leave-one-out cross-validation
    for (size_t testIndex = minTrainingSize; testIndex < data.size(); ++testIndex) {
        // Create training data efficiently
        const std::vector<Candlestick> trainingData(data.begin(), data.begin() + testIndex);
        
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
        } catch (const std::exception& e) {
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

std::vector<ValidationResult> Prediction::validateAllModels(const std::vector<Candlestick>& data) {
    std::vector<ValidationResult> results;
    results.reserve(3);
    
    // Define prediction functions using lambdas
    const std::vector<std::pair<PredictionFunction, int>> models = {
        {[](const std::vector<Candlestick>& d) { return predictLinearWithConfidence(d); }, 2},
        {[](const std::vector<Candlestick>& d) { return predictMovingAverageWithConfidence(d, 3); }, 1},
        {[](const std::vector<Candlestick>& d) { return predictHeuristicWithConfidence(d); }, 2}
    };
    
    for (const auto& [modelFunc, minSize] : models) {
        results.push_back(validateModel(data, modelFunc, minSize));
    }
    
    return results;
}

// Rolling predictions with improved efficiency

std::vector<PredictionResult> Prediction::generateRollingPredictions(
    const std::vector<Candlestick>& data,
    const PredictionFunction& modelFunction,
    int startIndex) {
    
    std::vector<PredictionResult> predictions;
    predictions.reserve(data.size() - startIndex);
    
    for (size_t i = startIndex; i < data.size(); ++i) {
        const std::vector<Candlestick> trainingData(data.begin(), data.begin() + i);
        predictions.push_back(modelFunction(trainingData));
    }
    
    return predictions;
}

// Enhanced display methods with better formatting

void Prediction::displayEnhancedPredictionResults(
    const std::vector<Candlestick>& data,
    TimeFrame timeframe,
    std::string_view country,
    int startYear, int endYear,
    const PredictionConfig& config) {
    
    if (data.empty()) {
        std::cout << "Cannot generate predictions: No data available.\n";
        return;
    }

    const std::string nextPeriod = determineNextPeriod(data, timeframe, endYear);
    
    // Display header with enhanced formatting
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "ENHANCED TEMPERATURE PREDICTION ANALYSIS\n";
    std::cout << std::string(70, '=') << "\n";
    std::cout << "Country: " << country << "\n";
    std::cout << "Data Range: " << startYear << "-" << endYear 
              << " (" << Utils::timeFrameToString(timeframe) << " data)\n";
    std::cout << "Data Points: " << data.size() << " periods\n";
    std::cout << "Predicting: " << nextPeriod << "\n\n";

    // Generate predictions using improved methods
    const auto results = std::vector<PredictionResult>{
        predictLinearWithConfidence(data),
        predictMovingAverageWithConfidence(data, config.movingAverageWindow),
        predictHeuristicWithConfidence(data)
    };

    displayPredictionSummary(results, nextPeriod);

    if (config.showValidation && data.size() >= 4) {
        std::cout << "\n" << std::string(50, '-') << "\n";
        std::cout << "CROSS-VALIDATION ANALYSIS\n";
        std::cout << std::string(50, '-') << "\n";
        const auto validationResults = validateAllModels(data);
        displayValidationResults(validationResults);
    }

    if (config.showChart && data.size() >= 4) {
        displayPredictionComparisonChart(data, timeframe, country, startYear, endYear, config);
    }
    
    std::cout << "\n";
}

void Prediction::displayPredictionSummary(
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
            std::cout << "   Assessment: " << getConfidenceLevel(result.confidenceMetric) << " confidence\n\n";
        } else {
            std::cout << "   Error: " << result.errorMessage << "\n\n";
        }
    }

    // Enhanced summary table
    displayPredictionTable(results, nextPeriod);
    displayRecommendation(results);
}

void Prediction::displayPredictionTable(
    const std::vector<PredictionResult>& results,
    const std::string& nextPeriod) {
    
    std::cout << "PREDICTION SUMMARY FOR " << nextPeriod << "\n";
    std::cout << std::string(80, '-') << "\n";
    std::cout << std::left << std::setw(25) << "Method" 
              << std::setw(12) << "Prediction" 
              << std::setw(15) << "Confidence"
              << std::setw(20) << "Reliability" << "\n";
    std::cout << std::string(80, '-') << "\n";
    
    for (const auto& result : results) {
        if (result.isValid) {
            std::cout << std::left << std::setw(25) << result.modelName
                      << std::setw(12) << (formatTemperature(result.predictionValue) + "°C")
                      << std::setw(15) << formatConfidence(result.confidenceMetric)
                      << std::setw(20) << getConfidenceLevel(result.confidenceMetric) << "\n";
        } else {
            std::cout << std::left << std::setw(25) << result.modelName 
                      << std::setw(50) << ("Error: " + result.errorMessage) << "\n";
        }
    }
    std::cout << "\n";
}

void Prediction::displayRecommendation(const std::vector<PredictionResult>& results) {
    const auto bestResult = std::max_element(results.begin(), results.end(),
        [](const PredictionResult& a, const PredictionResult& b) {
            if (!a.isValid) return true;
            if (!b.isValid) return false;
            return a.confidenceMetric < b.confidenceMetric;
        });

    std::cout << "CONFIDENCE-BASED RECOMMENDATION\n";
    std::cout << std::string(40, '-') << "\n";
    
    if (bestResult != results.end() && bestResult->isValid && bestResult->confidenceMetric > 0.0) {
        std::cout << "🎯 RECOMMENDED: " << bestResult->modelName << "\n";
        std::cout << "   Confidence: " << formatConfidence(bestResult->confidenceMetric) << "\n";
        std::cout << "   Reason: Highest confidence metric among available models\n";
    } else {
        std::cout << "⚠ No reliable model available - consider gathering more data\n";
    }
    std::cout << "\n";
}

void Prediction::displayPredictionComparisonChart(
    const std::vector<Candlestick>& data,
    TimeFrame timeframe,
    std::string_view country,
    int startYear, int endYear,
    const PredictionConfig& config) {
    
    if (data.size() < 4) {
        std::cout << "Insufficient data for prediction comparison chart (need at least 4 periods).\n";
        return;
    }

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "PREDICTION COMPARISON CHART\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << "Visual comparison for " << country << " (" << startYear << "-" << endYear << ")\n\n";

    // Generate all rolling predictions
    const auto allPredictions = generateAllRollingPredictions(data);
    
    // Display chart
    displayChartHeader();
    displayChartData(data, allPredictions, timeframe, startYear, config);
    displayChartFooter(data, config);
    displayChartLegend();
    displayAccuracyAnalysis(data, allPredictions);
}

std::vector<std::vector<PredictionResult>> Prediction::generateAllRollingPredictions(
    const std::vector<Candlestick>& data) {
    
    const std::vector<std::pair<PredictionFunction, int>> models = {
        {[](const auto& d) { return predictLinearWithConfidence(d); }, 2},
        {[](const auto& d) { return predictMovingAverageWithConfidence(d, 3); }, 1},
        {[](const auto& d) { return predictHeuristicWithConfidence(d); }, 2}
    };
    
    std::vector<std::vector<PredictionResult>> allPredictions;
    allPredictions.reserve(models.size());
    
    for (const auto& [modelFunc, startIdx] : models) {
        allPredictions.push_back(generateRollingPredictions(data, modelFunc, startIdx));
    }
    
    return allPredictions;
}

void Prediction::displayChartHeader() {
    std::cout << "Period    Actual   Linear   MovAvg   Heuris  |  Visual Comparison\n";
    std::cout << "          (°C)     (°C)     (°C)     (°C)    |  o=Actual ^=Linear #=MovAvg +=Heuristic\n";
    std::cout << std::string(78, '-') << "\n";
}

void Prediction::displayChartLegend() {
    std::cout << "\nLEGEND:\n";
    std::cout << "  o = Actual Temperature\n";
    std::cout << "  ^ = Linear Regression Prediction\n";
    std::cout << "  # = Moving Average Prediction\n";
    std::cout << "  + = Heuristic Model Prediction\n";
    std::cout << "  * = Multiple predictions at same position\n\n";
}

// Add other missing method implementations
void Prediction::displayChartData(
    const std::vector<Candlestick>& data,
    const std::vector<std::vector<PredictionResult>>& allPredictions,
    TimeFrame timeframe,
    int startYear,
    const PredictionConfig& config) {
    
    // Determine the range for scaling
    double minTemp = data[0].getClose();
    double maxTemp = data[0].getClose();
    
    for (const auto& candle : data) {
        minTemp = std::min(minTemp, candle.getClose());
        maxTemp = std::max(maxTemp, candle.getClose());
    }
    
    // Add some padding
    double range = maxTemp - minTemp;
    double padding = range * PredictionConfig::CHART_PADDING_RATIO;
    minTemp -= padding;
    maxTemp += padding;

    // Display each period with visual comparison
    for (size_t i = 0; i < data.size(); i++) {
        double actualTemp = data[i].getClose();
        
        // Format period label based on timeframe
        std::string periodLabel;
        if (timeframe == TimeFrame::Yearly) {
            periodLabel = std::to_string(startYear + i);
        } else if (timeframe == TimeFrame::Monthly) {
            int year = startYear + (i / 12);
            int month = (i % 12) + 1;
            std::ostringstream oss;
            oss << year << "-" << std::setfill('0') << std::setw(2) << month;
            periodLabel = oss.str();
        } else {
            periodLabel = "Period " + std::to_string(i + 1);
        }
        
        // Ensure period label is consistent width
        if (periodLabel.length() > 8) {
            periodLabel = periodLabel.substr(0, 8);
        }
        while (periodLabel.length() < 8) {
            periodLabel += " ";
        }

        std::cout << periodLabel << "  ";
        std::cout << std::fixed << std::setprecision(1) << std::setw(6) << actualTemp;

        // Show predictions if available
        std::string linearStr = "   -  ";
        std::string movingStr = "   -  ";
        std::string heuristicStr = "   -  ";

        if (allPredictions.size() > 0 && i >= 2 && (i - 2) < allPredictions[0].size() && allPredictions[0][i - 2].isValid) {
            linearStr = formatTemperature(allPredictions[0][i - 2].predictionValue, 1);
            while (linearStr.length() < 6) linearStr += " ";
        }
        
        if (allPredictions.size() > 1 && i >= 1 && (i - 1) < allPredictions[1].size() && allPredictions[1][i - 1].isValid) {
            movingStr = formatTemperature(allPredictions[1][i - 1].predictionValue, 1);
            while (movingStr.length() < 6) movingStr += " ";
        }
        
        if (allPredictions.size() > 2 && i >= 2 && (i - 2) < allPredictions[2].size() && allPredictions[2][i - 2].isValid) {
            heuristicStr = formatTemperature(allPredictions[2][i - 2].predictionValue, 1);
            while (heuristicStr.length() < 6) heuristicStr += " ";
        }

        std::cout << "  " << linearStr << "  " << movingStr << "  " << heuristicStr << "  |  ";

        // Create visual representation
        const int chartWidth = config.chartWidth;
        std::vector<char> chartLine(chartWidth, ' ');
        
        // Calculate positions for each value with bounds checking
        auto getPosition = [&](double value) -> int {
            if (maxTemp <= minTemp) return chartWidth / 2;
            double normalized = (value - minTemp) / (maxTemp - minTemp);
            int pos = static_cast<int>(normalized * (chartWidth - 1));
            return std::max(0, std::min(pos, chartWidth - 1));
        };

        int actualPos = getPosition(actualTemp);
        if (actualPos >= 0 && actualPos < chartWidth) {
            chartLine[actualPos] = 'o';
        }

        // Add predictions with bounds checking
        if (allPredictions.size() > 0 && i >= 2 && (i - 2) < allPredictions[0].size() && allPredictions[0][i - 2].isValid) {
            int linearPos = getPosition(allPredictions[0][i - 2].predictionValue);
            if (linearPos >= 0 && linearPos < chartWidth && linearPos != actualPos) {
                chartLine[linearPos] = '^';
            }
        }
        
        if (allPredictions.size() > 1 && i >= 1 && (i - 1) < allPredictions[1].size() && allPredictions[1][i - 1].isValid) {
            int movingPos = getPosition(allPredictions[1][i - 1].predictionValue);
            if (movingPos >= 0 && movingPos < chartWidth && movingPos != actualPos) {
                if (chartLine[movingPos] == ' ') {
                    chartLine[movingPos] = '#';
                } else {
                    chartLine[movingPos] = '*';
                }
            }
        }
        
        if (allPredictions.size() > 2 && i >= 2 && (i - 2) < allPredictions[2].size() && allPredictions[2][i - 2].isValid) {
            int heuristicPos = getPosition(allPredictions[2][i - 2].predictionValue);
            if (heuristicPos >= 0 && heuristicPos < chartWidth && heuristicPos != actualPos) {
                if (chartLine[heuristicPos] == ' ') {
                    chartLine[heuristicPos] = '+';
                } else {
                    chartLine[heuristicPos] = '*';
                }
            }
        }

        // Output the visual chart line
        for (char c : chartLine) {
            std::cout << c;
        }
        
        std::cout << "\n";
    }
}

void Prediction::displayChartFooter(
    const std::vector<Candlestick>& data,
    const PredictionConfig& config) {
    
    // Determine the range for scaling
    double minTemp = data[0].getClose();
    double maxTemp = data[0].getClose();
    
    for (const auto& candle : data) {
        minTemp = std::min(minTemp, candle.getClose());
        maxTemp = std::max(maxTemp, candle.getClose());
    }
    
    double range = maxTemp - minTemp;
    double padding = range * PredictionConfig::CHART_PADDING_RATIO;
    
    std::cout << std::string(78, '-') << "\n";
    std::cout << "Temperature Scale: ";
    std::cout << std::fixed << std::setprecision(1) << (minTemp + padding) << "°C";
    for (int i = 0; i < 35; i++) std::cout << " ";
    std::cout << (maxTemp - padding) << "°C\n\n";
}

void Prediction::displayAccuracyAnalysis(
    const std::vector<Candlestick>& data,
    const std::vector<std::vector<PredictionResult>>& allPredictions) {
    
    std::cout << "=== PREDICTION ACCURACY ANALYSIS ===\n";
    
    // Calculate errors for each model
    std::vector<std::vector<double>> allErrors(3);
    
    // Linear model errors (index 0)
    if (allPredictions.size() > 0) {
        for (size_t i = 2; i < data.size(); i++) {
            if ((i - 2) < allPredictions[0].size() && allPredictions[0][i - 2].isValid) {
                double actualTemp = data[i].getClose();
                double error = std::abs(actualTemp - allPredictions[0][i - 2].predictionValue);
                allErrors[0].push_back(error);
            }
        }
    }
    
    // Moving average errors (index 1)
    if (allPredictions.size() > 1) {
        for (size_t i = 1; i < data.size(); i++) {
            if ((i - 1) < allPredictions[1].size() && allPredictions[1][i - 1].isValid) {
                double actualTemp = data[i].getClose();
                double error = std::abs(actualTemp - allPredictions[1][i - 1].predictionValue);
                allErrors[1].push_back(error);
            }
        }
    }
    
    // Heuristic model errors (index 2)
    if (allPredictions.size() > 2) {
        for (size_t i = 2; i < data.size(); i++) {
            if ((i - 2) < allPredictions[2].size() && allPredictions[2][i - 2].isValid) {
                double actualTemp = data[i].getClose();
                double error = std::abs(actualTemp - allPredictions[2][i - 2].predictionValue);
                allErrors[2].push_back(error);
            }
        }
    }

    // Display accuracy summary
    std::cout << "Model Performance Summary:\n";
    std::cout << std::left << std::setw(20) << "Model" 
              << std::setw(15) << "Avg Error (°C)" 
              << std::setw(15) << "Max Error (°C)"
              << std::setw(15) << "Predictions" << "\n";
    std::cout << std::string(65, '-') << "\n";
    
    const std::vector<std::string> modelNames = {"Linear Regression", "Moving Average", "Heuristic Model"};
    
    for (size_t i = 0; i < allErrors.size() && i < modelNames.size(); i++) {
        if (!allErrors[i].empty()) {
            double avgError = calculateMean(allErrors[i]);
            double maxError = *std::max_element(allErrors[i].begin(), allErrors[i].end());
            std::cout << std::left << std::setw(20) << modelNames[i]
                      << std::setw(15) << formatDouble(avgError, 2)
                      << std::setw(15) << formatDouble(maxError, 2)
                      << std::setw(15) << allErrors[i].size() << "\n";
        }
    }

    std::cout << "\nNote: This chart shows rolling predictions where each prediction uses only\n";
    std::cout << "data available up to that point, simulating real-world forecasting conditions.\n";
    std::cout << "Lower average error indicates better historical prediction accuracy.\n\n";
}

// Utility and helper methods with improved efficiency

double Prediction::calculateRSquaredDetailed(
    const std::vector<Candlestick>& data, 
    double slope, double intercept) noexcept {
    
    if (data.size() < 2) return 0.0;
    
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
    
    if (totalSumSquares < EPSILON) return 0.0;
    
    const double rSquared = 1.0 - (residualSumSquares / totalSumSquares);
    return std::max(0.0, rSquared);
}

double Prediction::calculateStabilityConfidence(
    const std::vector<Candlestick>& data, 
    int windowSize) noexcept {
    
    if (data.size() < 2 || windowSize < 2) return 0.0;
    
    const auto start = std::max(0, static_cast<int>(data.size()) - windowSize);
    std::vector<double> recentValues;
    recentValues.reserve(windowSize);
    
    for (int i = start; i < static_cast<int>(data.size()); ++i) {
        recentValues.push_back(data[i].getClose());
    }
    
    if (recentValues.size() < 2) return 0.0;
    
    const double mean = calculateMean(recentValues);
    const double stdDev = calculateStandardDeviation(recentValues, mean);
    
    const double relativeVolatility = (mean > 0) ? stdDev / std::abs(mean) : stdDev;
    const double stabilityConfidence = 1.0 / (1.0 + relativeVolatility * PredictionConfig::STABILITY_SCALE_FACTOR);
    
    return std::clamp(stabilityConfidence, 0.0, 1.0);
}

double Prediction::calculateConsistencyConfidence(const std::vector<Candlestick>& data) noexcept {
    if (data.size() < 3) return 0.0;
    
    std::vector<double> changes;
    changes.reserve(data.size() - 1);
    
    for (size_t i = 1; i < data.size(); ++i) {
        changes.push_back(data[i].getClose() - data[i-1].getClose());
    }
    
    if (changes.size() < 2) return 0.0;
    
    const double changeMean = calculateMean(changes);
    const double changeStdDev = calculateStandardDeviation(changes, changeMean);
    
    const double changeVariability = changeStdDev / (std::abs(changeMean) + 1.0);
    const double consistencyConfidence = 1.0 / (1.0 + changeVariability);
    
    return std::clamp(consistencyConfidence, 0.0, 1.0);
}

// Input validation methods

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

// Display validation results

void Prediction::displayValidationResults(const std::vector<ValidationResult>& results) {
    if (results.empty()) {
        std::cout << "No validation results to display.\n";
        return;
    }

    std::vector<std::string> modelNames = {"Linear Regression", "Moving Average (3)", "Heuristic (Momentum)"};
    
    std::cout << "Cross-Validation Results:\n";
    std::cout << std::left << std::setw(20) << "Model" 
              << std::setw(12) << "MAE (°C)" 
              << std::setw(12) << "RMSE (°C)"
              << std::setw(12) << "Max Error"
              << std::setw(15) << "Success Rate" << "\n";
    std::cout << std::string(71, '-') << "\n";
    
    for (size_t i = 0; i < results.size() && i < modelNames.size(); i++) {
        const ValidationResult& result = results[i];
        
        if (result.isValid) {
            double rmse = result.getRMSE();
            double successRate = result.getSuccessRate();
            
            std::cout << std::left << std::setw(20) << modelNames[i]
                      << std::setw(12) << formatDouble(result.meanAbsoluteError, 2)
                      << std::setw(12) << formatDouble(rmse, 2)
                      << std::setw(12) << formatDouble(result.maxError, 2)
                      << std::setw(15) << (formatDouble(successRate, 1) + "%") << "\n";
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
        std::cout << "\n🏆 BEST VALIDATION PERFORMANCE: " << bestValidationModel 
                  << " (MAE: " << formatDouble(bestMAE, 2) << "°C)\n";
        std::cout << "   Note: Lower MAE indicates better historical prediction accuracy\n";
    }
}

// Formatting utility methods

std::string Prediction::formatTemperature(double temp, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << temp;
    return oss.str();
}

std::string Prediction::formatConfidence(double confidence, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << confidence;
    return oss.str();
}

std::string Prediction::formatDouble(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

std::string Prediction::getConfidenceLevel(double confidence) noexcept {
    if (confidence > PredictionConfig::HIGH_CONFIDENCE_THRESHOLD) {
        return "High";
    } else if (confidence > PredictionConfig::MODERATE_CONFIDENCE_THRESHOLD) {
        return "Moderate";
    } else {
        return "Low";
    }
}

std::string Prediction::getMonthName(int month) noexcept {
    static const std::vector<std::string> months = {
        "", "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    
    return (month >= 1 && month <= 12) ? months[month] : "Unknown";
}

// Chart utility methods

int Prediction::calculateChartPosition(
    double value, double minVal, double maxVal, int chartWidth) noexcept {
    
    if (maxVal <= minVal) return chartWidth / 2;
    
    const double normalized = (value - minVal) / (maxVal - minVal);
    const int pos = static_cast<int>(normalized * (chartWidth - 1));
    
    return std::clamp(pos, 0, chartWidth - 1);
}

void Prediction::generateChartLine(
    std::vector<char>& chartLine,
    const std::vector<std::pair<double, char>>& values,
    double minTemp, double maxTemp) noexcept {
    
    const int chartWidth = static_cast<int>(chartLine.size());
    
    for (const auto& [value, symbol] : values) {
        const int pos = calculateChartPosition(value, minTemp, maxTemp, chartWidth);
        if (pos >= 0 && pos < chartWidth) {
            if (chartLine[pos] == ' ') {
                chartLine[pos] = symbol;
            } else {
                chartLine[pos] = '*';  // Multiple values at same position
            }
        }
    }
}

// Legacy compatibility methods

double Prediction::predictLinear(const std::vector<Candlestick>& data) {
    const auto result = predictLinearWithConfidence(data);
    return result.isValid ? result.predictionValue : 0.0;
}

double Prediction::predictMovingAverage(const std::vector<Candlestick>& data, int windowSize) {
    const auto result = predictMovingAverageWithConfidence(data, windowSize);
    return result.isValid ? result.predictionValue : 0.0;
}

double Prediction::predictHeuristic(const std::vector<Candlestick>& data) {
    const auto result = predictHeuristicWithConfidence(data);
    return result.isValid ? result.predictionValue : 0.0;
}

void Prediction::displayPredictionResults(
    const std::vector<Candlestick>& data,
    TimeFrame timeframe,
    std::string_view country,
    int startYear, int endYear) {
    
    displayEnhancedPredictionResults(data, timeframe, country, startYear, endYear, {});
}

double Prediction::calculateLinearRSquared(const std::vector<Candlestick>& data) {
    const auto result = predictLinearWithConfidence(data);
    return result.isValid ? result.confidenceMetric : 0.0;
}

std::string Prediction::determineNextPeriod(
    const std::vector<Candlestick>& data, 
    TimeFrame timeframe, int endYear) {
    
    if (data.empty()) return "Unknown";
    
    const std::string& lastDate = data.back().getDate();
    
    switch (timeframe) {
        case TimeFrame::Yearly:
            return std::to_string(endYear + 1);
            
        case TimeFrame::Monthly:
            if (lastDate.length() >= 7) {
                const int year = std::stoi(lastDate.substr(0, 4));
                const int month = std::stoi(lastDate.substr(5, 2));
                
                if (month == 12) {
                    std::ostringstream oss;
                    oss << (year + 1) << "-01 (January " << (year + 1) << ")";
                    return oss.str();
                } else {
                    std::ostringstream oss;
                    oss << year << "-" << std::setfill('0') << std::setw(2) << (month + 1)
                        << " (" << getMonthName(month + 1) << " " << year << ")";
                    return oss.str();
                }
            }
            return "Next month after " + lastDate;
            
        case TimeFrame::Daily:
            return "Next day after " + lastDate;
            
        default:
            return "Next period after " + lastDate;
    }
}