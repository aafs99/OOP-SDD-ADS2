#include "Prediction.h"
#include "Common.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <numeric>

// Enhanced prediction methods with confidence metrics

PredictionResult Prediction::predictLinearWithConfidence(const std::vector<Candlestick>& data) {
    const std::string modelName = "Linear Regression";
    
    // Check for sufficient data
    if (data.size() < 2) {
        return PredictionResult("Insufficient data (need at least 2 points)", modelName);
    }
    
    int n = data.size();
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    
    // Calculate sums for x, y, xy, x²
    for (int i = 0; i < n; i++) {
        double x = static_cast<double>(i);  // Time index
        double y = data[i].getClose();      // Temperature
        
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }
    
    // Calculate slope and intercept using least squares method
    double denominator = n * sumX2 - sumX * sumX;
    if (std::abs(denominator) < 1e-10) {
        // Degenerate case - return mean with low confidence
        double prediction = sumY / n;
        return PredictionResult(prediction, 0.0, modelName, 
                              "R² = 0.0 (no linear trend detected)");
    }
    
    double slope = (n * sumXY - sumX * sumY) / denominator;
    double intercept = (sumY - slope * sumX) / n;
    
    // Calculate prediction for next period
    double nextX = static_cast<double>(n);
    double prediction = slope * nextX + intercept;
    
    // Calculate confidence metric (R²)
    double rSquared = calculateRSquaredDetailed(data, slope, intercept);
    
    std::string confDesc = "R² = " + std::to_string(rSquared).substr(0, 5) + 
                          " (coefficient of determination: higher = better fit)";
    
    return PredictionResult(prediction, rSquared, modelName, confDesc);
}

PredictionResult Prediction::predictMovingAverageWithConfidence(const std::vector<Candlestick>& data, 
                                                               int windowSize) {
    std::string modelName = "Moving Average (" + std::to_string(windowSize) + "-period)";
    
    if (data.empty()) {
        return PredictionResult("No data available", modelName);
    }
    
    if (windowSize <= 0) windowSize = 3;  // Default window
    int actualWindowSize = std::min(windowSize, static_cast<int>(data.size()));
    
    // Calculate moving average prediction
    double sum = 0.0;
    for (int i = data.size() - actualWindowSize; i < static_cast<int>(data.size()); i++) {
        sum += data[i].getClose();
    }
    double prediction = sum / actualWindowSize;
    
    // Calculate stability confidence metric
    double stabilityConfidence = calculateStabilityConfidence(data, actualWindowSize);
    
    std::string confDesc = "Stability = " + std::to_string(stabilityConfidence).substr(0, 5) +
                          " (based on inverse volatility: higher = more stable)";
    
    return PredictionResult(prediction, stabilityConfidence, modelName, confDesc);
}

PredictionResult Prediction::predictHeuristicWithConfidence(const std::vector<Candlestick>& data) {
    const std::string modelName = "Heuristic (Momentum)";
    
    if (data.empty()) {
        return PredictionResult("No data available", modelName);
    }
    
    if (data.size() == 1) {
        return PredictionResult(data[0].getClose(), 0.0, modelName,
                              "Single data point (no momentum to calculate)");
    }
    
    // Calculate momentum prediction
    double last = data.back().getClose();
    double secondLast = data[data.size() - 2].getClose();
    double change = last - secondLast;
    double prediction = last + change;
    
    // Calculate consistency confidence metric
    double consistencyConfidence = calculateConsistencyConfidence(data);
    
    std::string confDesc = "Consistency = " + std::to_string(consistencyConfidence).substr(0, 5) +
                          " (trend consistency: higher = more reliable momentum)";
    
    return PredictionResult(prediction, consistencyConfidence, modelName, confDesc);
}

// Cross-validation implementation

ValidationResult Prediction::validateModel(const std::vector<Candlestick>& data,
                                          PredictionFunction modelFunction,
                                          int minTrainingSize) {
    ValidationResult result;
    
    // Check for sufficient data
    if (static_cast<int>(data.size()) < minTrainingSize + 1) {
        result.errorMessage = "Insufficient data for validation (need at least " + 
                             std::to_string(minTrainingSize + 1) + " points)";
        return result;
    }
    
    std::vector<double> errors;
    std::vector<double> squaredErrors;
    
    // Perform leave-one-out cross-validation
    for (size_t testIndex = minTrainingSize; testIndex < data.size(); testIndex++) {
        // Create training data from all previous points
        std::vector<Candlestick> trainingData(data.begin(), data.begin() + testIndex);
        
        // Generate prediction using model function
        PredictionResult predResult = modelFunction(trainingData);
        
        if (predResult.isValid) {
            // Compare with actual value
            double actualValue = data[testIndex].getClose();
            double error = std::abs(predResult.predictionValue - actualValue);
            double squaredError = (predResult.predictionValue - actualValue) * 
                                 (predResult.predictionValue - actualValue);
            
            errors.push_back(error);
            squaredErrors.push_back(squaredError);
            result.validPredictions++;
        }
        result.totalAttempts++;
    }
    
    if (errors.empty()) {
        result.errorMessage = "No valid predictions generated during validation";
        return result;
    }
    
    // Calculate statistical metrics
    result.meanAbsoluteError = calculateMean(errors);
    result.meanSquaredError = calculateMean(squaredErrors);
    result.maxError = *std::max_element(errors.begin(), errors.end());
    result.minError = *std::min_element(errors.begin(), errors.end());
    result.isValid = true;
    
    return result;
}

std::vector<ValidationResult> Prediction::validateAllModels(const std::vector<Candlestick>& data) {
    std::vector<ValidationResult> results;
    
    // Validate Linear Regression
    auto linearFunction = [](const std::vector<Candlestick>& d) -> PredictionResult {
        return predictLinearWithConfidence(d);
    };
    ValidationResult linearResult = validateModel(data, linearFunction, 2);
    results.push_back(linearResult);
    
    // Validate Moving Average (3-period)
    auto movingAvgFunction = [](const std::vector<Candlestick>& d) -> PredictionResult {
        return predictMovingAverageWithConfidence(d, 3);
    };
    ValidationResult movingAvgResult = validateModel(data, movingAvgFunction, 1);
    results.push_back(movingAvgResult);
    
    // Validate Heuristic Model
    auto heuristicFunction = [](const std::vector<Candlestick>& d) -> PredictionResult {
        return predictHeuristicWithConfidence(d);
    };
    ValidationResult heuristicResult = validateModel(data, heuristicFunction, 2);
    results.push_back(heuristicResult);
    
    return results;
}

// Rolling predictions for chart comparison

std::vector<PredictionResult> Prediction::generateRollingPredictions(
    const std::vector<Candlestick>& data,
    PredictionFunction modelFunction,
    int startIndex) {
    
    std::vector<PredictionResult> predictions;
    
    // Generate predictions for each period using only previous data
    for (size_t i = startIndex; i < data.size(); i++) {
        // Create training data from all previous points
        std::vector<Candlestick> trainingData(data.begin(), data.begin() + i);
        
        // Generate prediction for period i
        PredictionResult prediction = modelFunction(trainingData);
        predictions.push_back(prediction);
    }
    
    return predictions;
}

void Prediction::displayPredictionComparisonChart(const std::vector<Candlestick>& data,
                                                TimeFrame timeframe,
                                                const std::string& country,
                                                int startYear, int endYear) {
    if (data.size() < 4) {
        std::cout << "Insufficient data for prediction comparison chart (need at least 4 periods).\n";
        return;
    }

    std::cout << "\n=== PREDICTION COMPARISON CHART ===\n";
    std::cout << "Generating rolling predictions for visual comparison...\n\n";

    // Generate rolling predictions for each model
    auto linearFunction = [](const std::vector<Candlestick>& d) -> PredictionResult {
        return predictLinearWithConfidence(d);
    };
    auto movingAvgFunction = [](const std::vector<Candlestick>& d) -> PredictionResult {
        return predictMovingAverageWithConfidence(d, 3);
    };
    auto heuristicFunction = [](const std::vector<Candlestick>& d) -> PredictionResult {
        return predictHeuristicWithConfidence(d);
    };

    std::vector<PredictionResult> linearPredictions = generateRollingPredictions(data, linearFunction, 2);
    std::vector<PredictionResult> movingAvgPredictions = generateRollingPredictions(data, movingAvgFunction, 1);
    std::vector<PredictionResult> heuristicPredictions = generateRollingPredictions(data, heuristicFunction, 2);

    // Create the comparison chart
    std::cout << "Figure 4: Temperature Predictions vs Actual Values (" << country << ", " 
              << startYear << "-" << endYear << ")\n\n";

    // Chart header
    std::cout << "Period    Actual   Linear   MovAvg   Heuris  |  Visual Comparison\n";
    std::cout << "          (°C)     (°C)     (°C)     (°C)    |  ●=Actual ▲=Linear ■=MovAvg ♦=Heuristic\n";
    std::cout << std::string(78, '-') << "\n";

    // Determine the range for scaling
    double minTemp = data[0].getClose();
    double maxTemp = data[0].getClose();
    
    for (const auto& candle : data) {
        minTemp = std::min(minTemp, candle.getClose());
        maxTemp = std::max(maxTemp, candle.getClose());
    }
    
    // Add some padding
    double range = maxTemp - minTemp;
    double padding = range * 0.1;
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
            periodLabel = std::to_string(year) + "-" + (month < 10 ? "0" : "") + std::to_string(month);
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

        if (i >= 2 && (i - 2) < linearPredictions.size() && linearPredictions[i - 2].isValid) {
            linearStr = std::to_string(linearPredictions[i - 2].predictionValue).substr(0, 5);
            while (linearStr.length() < 6) linearStr += " ";
        }
        
        if (i >= 1 && (i - 1) < movingAvgPredictions.size() && movingAvgPredictions[i - 1].isValid) {
            movingStr = std::to_string(movingAvgPredictions[i - 1].predictionValue).substr(0, 5);
            while (movingStr.length() < 6) movingStr += " ";
        }
        
        if (i >= 2 && (i - 2) < heuristicPredictions.size() && heuristicPredictions[i - 2].isValid) {
            heuristicStr = std::to_string(heuristicPredictions[i - 2].predictionValue).substr(0, 5);
            while (heuristicStr.length() < 6) heuristicStr += " ";
        }

        std::cout << "  " << linearStr << "  " << movingStr << "  " << heuristicStr << "  |  ";

        // Create visual representation
        const int chartWidth = 20;
        std::vector<char> chartLine(chartWidth, ' ');
        
        // Calculate positions for each value
        auto getPosition = [&](double value) -> int {
            if (maxTemp == minTemp) return chartWidth / 2;
            double normalized = (value - minTemp) / (maxTemp - minTemp);
            return static_cast<int>(normalized * (chartWidth - 1));
        };

        int actualPos = getPosition(actualTemp);
        chartLine[actualPos] = 'o';  // Actual temperature

        // Add predictions
        if (i >= 2 && (i - 2) < linearPredictions.size() && linearPredictions[i - 2].isValid) {
            int linearPos = getPosition(linearPredictions[i - 2].predictionValue);
            if (linearPos != actualPos && linearPos >= 0 && linearPos < chartWidth) {
                chartLine[linearPos] = '^';  // Linear prediction
            }
        }
        
        if (i >= 1 && (i - 1) < movingAvgPredictions.size() && movingAvgPredictions[i - 1].isValid) {
            int movingPos = getPosition(movingAvgPredictions[i - 1].predictionValue);
            if (movingPos != actualPos && movingPos >= 0 && movingPos < chartWidth) {
                if (chartLine[movingPos] == ' ') {
                    chartLine[movingPos] = '#';  // Moving average prediction
                } else {
                    chartLine[movingPos] = '*';  // Multiple predictions at same position
                }
            }
        }
        
        if (i >= 2 && (i - 2) < heuristicPredictions.size() && heuristicPredictions[i - 2].isValid) {
            int heuristicPos = getPosition(heuristicPredictions[i - 2].predictionValue);
            if (heuristicPos != actualPos && heuristicPos >= 0 && heuristicPos < chartWidth) {
                if (chartLine[heuristicPos] == ' ') {
                    chartLine[heuristicPos] = '+';  // Heuristic prediction
                } else {
                    chartLine[heuristicPos] = '*';  // Multiple predictions at same position
                }
            }
        }

        // Output the visual chart line
        for (char c : chartLine) {
            std::cout << c;
        }
        
        std::cout << "\n";
    }

    // Chart footer with temperature scale
    std::cout << std::string(78, '-') << "\n";
    std::cout << "Temperature Scale: ";
    std::cout << std::fixed << std::setprecision(1) << minTemp + padding << "°C";
    for (int i = 0; i < 35; i++) std::cout << " ";
    std::cout << maxTemp - padding << "°C\n\n";

    // Legend
    std::cout << "LEGEND:\n";
    std::cout << "  o = Actual Temperature (black in color version)\n";
    std::cout << "  ^ = Linear Regression Prediction (blue in color version)\n";
    std::cout << "  # = Moving Average Prediction (green in color version)\n";
    std::cout << "  + = Heuristic Model Prediction (red in color version)\n";
    std::cout << "  * = Multiple predictions at same position\n\n";

    // Calculate and display prediction accuracy statistics
    std::cout << "=== PREDICTION ACCURACY ANALYSIS ===\n";
    
    // Calculate errors for each model
    std::vector<double> linearErrors, movingAvgErrors, heuristicErrors;
    
    for (size_t i = 2; i < data.size(); i++) {
        double actualTemp = data[i].getClose();
        
        // Linear model errors
        if ((i - 2) < linearPredictions.size() && linearPredictions[i - 2].isValid) {
            double error = std::abs(actualTemp - linearPredictions[i - 2].predictionValue);
            linearErrors.push_back(error);
        }
        
        // Heuristic model errors
        if ((i - 2) < heuristicPredictions.size() && heuristicPredictions[i - 2].isValid) {
            double error = std::abs(actualTemp - heuristicPredictions[i - 2].predictionValue);
            heuristicErrors.push_back(error);
        }
    }
    
    for (size_t i = 1; i < data.size(); i++) {
        double actualTemp = data[i].getClose();
        
        // Moving average errors
        if ((i - 1) < movingAvgPredictions.size() && movingAvgPredictions[i - 1].isValid) {
            double error = std::abs(actualTemp - movingAvgPredictions[i - 1].predictionValue);
            movingAvgErrors.push_back(error);
        }
    }

    // Display accuracy summary
    std::cout << "Model Performance Summary:\n";
    std::cout << std::left << std::setw(20) << "Model" 
              << std::setw(15) << "Avg Error (°C)" 
              << std::setw(15) << "Max Error (°C)"
              << std::setw(15) << "Predictions" << "\n";
    std::cout << std::string(65, '-') << "\n";
    
    if (!linearErrors.empty()) {
        double avgError = calculateMean(linearErrors);
        double maxError = *std::max_element(linearErrors.begin(), linearErrors.end());
        std::cout << std::left << std::setw(20) << "Linear Regression"
                  << std::setw(15) << (std::to_string(avgError).substr(0, 5))
                  << std::setw(15) << (std::to_string(maxError).substr(0, 5))
                  << std::setw(15) << linearErrors.size() << "\n";
    }
    
    if (!movingAvgErrors.empty()) {
        double avgError = calculateMean(movingAvgErrors);
        double maxError = *std::max_element(movingAvgErrors.begin(), movingAvgErrors.end());
        std::cout << std::left << std::setw(20) << "Moving Average"
                  << std::setw(15) << (std::to_string(avgError).substr(0, 5))
                  << std::setw(15) << (std::to_string(maxError).substr(0, 5))
                  << std::setw(15) << movingAvgErrors.size() << "\n";
    }
    
    if (!heuristicErrors.empty()) {
        double avgError = calculateMean(heuristicErrors);
        double maxError = *std::max_element(heuristicErrors.begin(), heuristicErrors.end());
        std::cout << std::left << std::setw(20) << "Heuristic Model"
                  << std::setw(15) << (std::to_string(avgError).substr(0, 5))
                  << std::setw(15) << (std::to_string(maxError).substr(0, 5))
                  << std::setw(15) << heuristicErrors.size() << "\n";
    }

    std::cout << "\nNote: This chart shows rolling predictions where each prediction uses only\n";
    std::cout << "data available up to that point, simulating real-world forecasting conditions.\n";
    std::cout << "Lower average error indicates better historical prediction accuracy.\n\n";
}

// Enhanced display methods

void Prediction::displayEnhancedPredictionResults(const std::vector<Candlestick>& data,
                                                 TimeFrame timeframe,
                                                 const std::string& country,
                                                 int startYear, int endYear,
                                                 bool showValidation) {
    if (data.empty()) {
        std::cout << "Cannot generate predictions: No data available.\n";
        return;
    }

    std::string nextPeriod = determineNextPeriod(data, timeframe, endYear);
    
    std::cout << "\n=== ENHANCED TASK 4: Temperature Prediction with Confidence Metrics ===\n";
    std::cout << "Country: " << country << "\n";
    std::cout << "Data Range: " << startYear << "-" << endYear << " (" 
              << Utils::timeFrameToString(timeframe) << " data)\n";
    std::cout << "Data Points: " << data.size() << " periods\n";
    std::cout << "Predicting: " << nextPeriod << "\n\n";

    // Generate enhanced predictions
    PredictionResult linearResult = predictLinearWithConfidence(data);
    PredictionResult movingAvgResult = predictMovingAverageWithConfidence(data, 3);
    PredictionResult heuristicResult = predictHeuristicWithConfidence(data);

    std::cout << "=== Enhanced Prediction Methods with Confidence Metrics ===\n\n";

    // Display Linear Regression
    std::cout << "1. LINEAR REGRESSION MODEL\n";
    std::cout << "   Method: Least squares trend line fitting\n";
    std::cout << "   Calculation: y = mx + b, where m = slope, b = intercept\n";
    std::cout << "   Justification: Identifies long-term temperature trends from historical data\n";
    if (linearResult.isValid) {
        std::cout << "   Result: " << std::fixed << std::setprecision(2) 
                  << linearResult.predictionValue << "°C\n";
        std::cout << "   Confidence: " << linearResult.confidenceDescription << "\n";
        if (linearResult.confidenceMetric > 0.7) {
            std::cout << "   Assessment: ✓ HIGH confidence - Strong linear trend detected\n\n";
        } else if (linearResult.confidenceMetric > 0.4) {
            std::cout << "   Assessment: ⚠ MODERATE confidence - Weak linear trend\n\n";
        } else {
            std::cout << "   Assessment: ⚠ LOW confidence - No clear linear trend\n\n";
        }
    } else {
        std::cout << "   Error: " << linearResult.errorMessage << "\n\n";
    }

    // Display Moving Average
    std::cout << "2. MOVING AVERAGE MODEL (3-period)\n";
    std::cout << "   Method: Average of recent temperature values\n";
    std::cout << "   Calculation: (T₁ + T₂ + T₃) / 3\n";
    std::cout << "   Justification: Smooths short-term fluctuations, provides stable forecast\n";
    if (movingAvgResult.isValid) {
        std::cout << "   Result: " << std::fixed << std::setprecision(2) 
                  << movingAvgResult.predictionValue << "°C\n";
        std::cout << "   Confidence: " << movingAvgResult.confidenceDescription << "\n";
        if (movingAvgResult.confidenceMetric > 0.7) {
            std::cout << "   Assessment: ✓ HIGH stability - Low recent volatility\n\n";
        } else if (movingAvgResult.confidenceMetric > 0.4) {
            std::cout << "   Assessment: ⚠ MODERATE stability - Some volatility present\n\n";
        } else {
            std::cout << "   Assessment: ⚠ LOW stability - High recent volatility\n\n";
        }
    } else {
        std::cout << "   Error: " << movingAvgResult.errorMessage << "\n\n";
    }

    // Display Heuristic Model
    std::cout << "3. HEURISTIC (MOMENTUM) MODEL\n";
    std::cout << "   Method: Projects recent temperature change forward\n";
    std::cout << "   Calculation: Next = Current + (Current - Previous)\n";
    std::cout << "   Justification: Assumes recent momentum will continue\n";
    if (heuristicResult.isValid) {
        std::cout << "   Result: " << std::fixed << std::setprecision(2) 
                  << heuristicResult.predictionValue << "°C\n";
        std::cout << "   Confidence: " << heuristicResult.confidenceDescription << "\n";
        if (heuristicResult.confidenceMetric > 0.7) {
            std::cout << "   Assessment: ✓ HIGH consistency - Reliable momentum trend\n\n";
        } else if (heuristicResult.confidenceMetric > 0.4) {
            std::cout << "   Assessment: ⚠ MODERATE consistency - Some trend variability\n\n";
        } else {
            std::cout << "   Assessment: ⚠ LOW consistency - Highly variable trends\n\n";
        }
    } else {
        std::cout << "   Error: " << heuristicResult.errorMessage << "\n\n";
    }

    // Enhanced summary with confidence-based recommendations
    std::cout << "=== Enhanced Prediction Summary for " << nextPeriod << " ===\n";
    std::cout << std::left << std::setw(25) << "Method" 
              << std::setw(12) << "Prediction" 
              << std::setw(15) << "Confidence"
              << std::setw(20) << "Reliability" << "\n";
    std::cout << std::string(72, '-') << "\n";
    
    if (linearResult.isValid) {
        std::string reliability = linearResult.confidenceMetric > 0.7 ? "High" :
                                linearResult.confidenceMetric > 0.4 ? "Moderate" : "Low";
        std::cout << std::left << std::setw(25) << "Linear Regression" 
                  << std::setw(12) << (std::to_string(linearResult.predictionValue).substr(0,5) + "°C")
                  << std::setw(15) << (std::to_string(linearResult.confidenceMetric).substr(0,5))
                  << std::setw(20) << reliability << "\n";
    }
    
    if (movingAvgResult.isValid) {
        std::string reliability = movingAvgResult.confidenceMetric > 0.7 ? "High" :
                                movingAvgResult.confidenceMetric > 0.4 ? "Moderate" : "Low";
        std::cout << std::left << std::setw(25) << "Moving Average (3)" 
                  << std::setw(12) << (std::to_string(movingAvgResult.predictionValue).substr(0,5) + "°C")
                  << std::setw(15) << (std::to_string(movingAvgResult.confidenceMetric).substr(0,5))
                  << std::setw(20) << reliability << "\n";
    }
    
    if (heuristicResult.isValid) {
        std::string reliability = heuristicResult.confidenceMetric > 0.7 ? "High" :
                                heuristicResult.confidenceMetric > 0.4 ? "Moderate" : "Low";
        std::cout << std::left << std::setw(25) << "Heuristic (Momentum)" 
                  << std::setw(12) << (std::to_string(heuristicResult.predictionValue).substr(0,5) + "°C")
                  << std::setw(15) << (std::to_string(heuristicResult.confidenceMetric).substr(0,5))
                  << std::setw(20) << reliability << "\n";
    }

    // Confidence-based recommendation
    std::cout << "\n=== Confidence-Based Recommendation ===\n";
    double bestConfidence = 0.0;
    std::string bestModel = "None";
    
    if (linearResult.isValid && linearResult.confidenceMetric > bestConfidence) {
        bestConfidence = linearResult.confidenceMetric;
        bestModel = "Linear Regression (R² = " + std::to_string(linearResult.confidenceMetric).substr(0,4) + ")";
    }
    if (movingAvgResult.isValid && movingAvgResult.confidenceMetric > bestConfidence) {
        bestConfidence = movingAvgResult.confidenceMetric;
        bestModel = "Moving Average (Stability = " + std::to_string(movingAvgResult.confidenceMetric).substr(0,4) + ")";
    }
    if (heuristicResult.isValid && heuristicResult.confidenceMetric > bestConfidence) {
        bestConfidence = heuristicResult.confidenceMetric;
        bestModel = "Heuristic Model (Consistency = " + std::to_string(heuristicResult.confidenceMetric).substr(0,4) + ")";
    }
    
    if (bestConfidence > 0.0) {
        std::cout << "🎯 RECOMMENDED: " << bestModel << "\n";
        std::cout << "   Reason: Highest confidence metric among available models\n";
    } else {
        std::cout << "⚠ No reliable model available - consider gathering more data\n";
    }

    // Cross-validation analysis if requested
    if (showValidation && data.size() >= 4) {
        std::cout << "\n=== Cross-Validation Analysis ===\n";
        std::cout << "Performing leave-one-out cross-validation to assess model accuracy...\n\n";
        
        std::vector<ValidationResult> validationResults = validateAllModels(data);
        displayValidationResults(validationResults);
    }

    // Offer prediction comparison chart
    if (data.size() >= 4) {
        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << "PREDICTION COMPARISON CHART AVAILABLE\n";
        std::cout << std::string(60, '=') << "\n";
        std::cout << "Generate a visual chart comparing actual temperatures with predictions\n";
        std::cout << "from all three models across the historical period.\n";
        std::cout << "\nThis creates 'Figure 4' style visualization showing:\n";
        std::cout << "• Actual temperatures (o)\n";
        std::cout << "• Linear model predictions (^)\n";
        std::cout << "• Moving average predictions (#)\n";
        std::cout << "• Heuristic model predictions (+)\n\n";
        
        std::string choice;
        std::cout << "Would you like to generate the prediction comparison chart? (y/n): ";
        std::cin >> choice;
        std::transform(choice.begin(), choice.end(), choice.begin(), ::tolower);
        
        if (choice == "y" || choice == "yes") {
            displayPredictionComparisonChart(data, timeframe, country, startYear, endYear);
        }
    }
    
    std::cout << "\n";
}

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
            double rmse = std::sqrt(result.meanSquaredError);
            double successRate = result.totalAttempts > 0 ? 
                               (static_cast<double>(result.validPredictions) / result.totalAttempts) * 100.0 : 0.0;
            
            std::cout << std::left << std::setw(20) << modelNames[i]
                      << std::setw(12) << (std::to_string(result.meanAbsoluteError).substr(0,5))
                      << std::setw(12) << (std::to_string(rmse).substr(0,5))
                      << std::setw(12) << (std::to_string(result.maxError).substr(0,5))
                      << std::setw(15) << (std::to_string(successRate).substr(0,5) + "%") << "\n";
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
                  << " (MAE: " << std::fixed << std::setprecision(2) << bestMAE << "°C)\n";
        std::cout << "   Note: Lower MAE indicates better historical prediction accuracy\n";
    }
}

// Private helper methods implementation

double Prediction::calculateRSquaredDetailed(const std::vector<Candlestick>& data, 
                                           double slope, double intercept) {
    if (data.size() < 2) return 0.0;
    
    // Calculate mean of y values
    double sumY = 0.0;
    for (const auto& candle : data) {
        sumY += candle.getClose();
    }
    double meanY = sumY / data.size();
    
    // Calculate total sum of squares and residual sum of squares
    double totalSumSquares = 0.0;
    double residualSumSquares = 0.0;
    
    for (size_t i = 0; i < data.size(); i++) {
        double actualY = data[i].getClose();
        double predictedY = slope * static_cast<double>(i) + intercept;
        
        totalSumSquares += (actualY - meanY) * (actualY - meanY);
        residualSumSquares += (actualY - predictedY) * (actualY - predictedY);
    }
    
    // Calculate R² = 1 - (residual SS / total SS)
    if (totalSumSquares < 1e-10) return 0.0;  // Avoid division by zero
    
    double rSquared = 1.0 - (residualSumSquares / totalSumSquares);
    return std::max(0.0, rSquared);  // Ensure non-negative
}

double Prediction::calculateStabilityConfidence(const std::vector<Candlestick>& data, 
                                               int windowSize) {
    if (data.size() < 2 || windowSize < 2) return 0.0;
    
    // Calculate recent volatility
    std::vector<double> recentValues;
    int start = std::max(0, static_cast<int>(data.size()) - windowSize);
    
    for (int i = start; i < static_cast<int>(data.size()); i++) {
        recentValues.push_back(data[i].getClose());
    }
    
    if (recentValues.size() < 2) return 0.0;
    
    double mean = calculateMean(recentValues);
    double stdDev = calculateStandardDeviation(recentValues, mean);
    
    // Convert to stability confidence (inverse of relative volatility)
    // Higher volatility = lower confidence
    double relativeVolatility = (mean > 0) ? stdDev / std::abs(mean) : stdDev;
    double stabilityConfidence = 1.0 / (1.0 + relativeVolatility * 5.0);  // Scale factor for reasonable range
    
    return std::min(1.0, std::max(0.0, stabilityConfidence));
}

double Prediction::calculateConsistencyConfidence(const std::vector<Candlestick>& data) {
    if (data.size() < 3) return 0.0;
    
    // Calculate recent trend changes
    std::vector<double> changes;
    for (size_t i = 1; i < data.size(); i++) {
        double change = data[i].getClose() - data[i-1].getClose();
        changes.push_back(change);
    }
    
    if (changes.size() < 2) return 0.0;
    
    // Calculate consistency as inverse of change variability
    double changeMean = calculateMean(changes);
    double changeStdDev = calculateStandardDeviation(changes, changeMean);
    
    // Convert to consistency confidence
    double changeVariability = changeStdDev / (std::abs(changeMean) + 1.0);  // Add 1 to avoid division by zero
    double consistencyConfidence = 1.0 / (1.0 + changeVariability);
    
    return std::min(1.0, std::max(0.0, consistencyConfidence));
}

double Prediction::calculateMean(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
}

double Prediction::calculateStandardDeviation(const std::vector<double>& values, double mean) {
    if (values.size() < 2) return 0.0;
    
    double sumSquaredDiffs = 0.0;
    for (double value : values) {
        double diff = value - mean;
        sumSquaredDiffs += diff * diff;
    }
    
    return std::sqrt(sumSquaredDiffs / (values.size() - 1));
}

// Legacy methods implementation (for backward compatibility)

double Prediction::predictLinear(const std::vector<Candlestick>& data) {
    PredictionResult result = predictLinearWithConfidence(data);
    return result.isValid ? result.predictionValue : 0.0;
}

double Prediction::predictMovingAverage(const std::vector<Candlestick>& data, int windowSize) {
    PredictionResult result = predictMovingAverageWithConfidence(data, windowSize);
    return result.isValid ? result.predictionValue : 0.0;
}

double Prediction::predictHeuristic(const std::vector<Candlestick>& data) {
    PredictionResult result = predictHeuristicWithConfidence(data);
    return result.isValid ? result.predictionValue : 0.0;
}

void Prediction::displayPredictionResults(const std::vector<Candlestick>& data,
                                        TimeFrame timeframe,
                                        const std::string& country,
                                        int startYear, int endYear) {
    // Use the enhanced version without cross-validation for backward compatibility
    displayEnhancedPredictionResults(data, timeframe, country, startYear, endYear, false);
}

double Prediction::calculateLinearRSquared(const std::vector<Candlestick>& data) {
    PredictionResult result = predictLinearWithConfidence(data);
    return result.isValid ? result.confidenceMetric : 0.0;
}

std::string Prediction::determineNextPeriod(const std::vector<Candlestick>& data, 
                                           TimeFrame timeframe, int endYear) {
    if (data.empty()) return "Unknown";
    
    std::string lastDate = data.back().getDate();
    
    switch (timeframe) {
        case TimeFrame::Yearly:
            return std::to_string(endYear + 1);
            
        case TimeFrame::Monthly:
            if (lastDate.length() >= 7) {
                int year = std::stoi(lastDate.substr(0, 4));
                int month = std::stoi(lastDate.substr(5, 2));
                
                if (month == 12) {
                    return std::to_string(year + 1) + "-01 (January " + std::to_string(year + 1) + ")";
                } else {
                    return std::to_string(year) + "-" + 
                           (month + 1 < 10 ? "0" : "") + std::to_string(month + 1) +
                           " (" + getMonthName(month + 1) + " " + std::to_string(year) + ")";
                }
            }
            return "Next month after " + lastDate;
            
        case TimeFrame::Daily:
            return "Next day after " + lastDate;
            
        default:
            return "Next period after " + lastDate;
    }
}

std::string Prediction::getMonthName(int month) {
    const std::string months[] = {
        "", "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    
    if (month >= 1 && month <= 12) {
        return months[month];
    }
    return "Unknown";
}