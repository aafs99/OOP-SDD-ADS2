#include "PredictionChart.h"
#include "Prediction.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>

/*self written code start (code built with reference to : 
 https://github.com/KC1922/StockMarketDisplay-CPP/tree/main/StockProjectCPP
 https://www.youtube.com/watch?v=lqeGdfprKn0&list=PLiPFKCVZWHKqk1TSSQTBcPSxZEjBT9E88&index=7
*/
//! TDL Output looks messy, replace txt with ANSI codes for better readability
void PredictionChart::displayVerticalChart(
    const std::vector<Candlestick>& data,
    TimeFrame timeframe,
    std::string_view country,
    int startYear, int endYear) {
    
    if (data.size() < Constants::MIN_LINEAR_DATA_SIZE + Constants::MIN_HEURISTIC_DATA_SIZE) {
        std::cout << "Insufficient data for prediction chart (need at least " 
                  << (Constants::MIN_LINEAR_DATA_SIZE + Constants::MIN_HEURISTIC_DATA_SIZE) 
                  << " periods).\n";
        return;
    }

    // Display chart header
    displayChartHeader(data, country, startYear, endYear);

    // Generate all rolling predictions
    const auto allPredictions = generateAllRollingPredictions(data);
    
    // Display the vertical chart
    displayChart(data, allPredictions, timeframe, startYear);
    
    // Display chart footer and accuracy analysis
    displayChartFooter();
    displayAccuracyAnalysis(data, allPredictions);
}

void PredictionChart::displayChartHeader(const std::vector<Candlestick>& data, std::string_view country, 
                                        int startYear, int endYear) {
    std::cout << "\n" << std::string(Constants::SECTION_SEPARATOR_WIDTH_60, '=') << "\n";
    std::cout << "PREDICTION COMPARISON CHART\n";
    std::cout << std::string(Constants::SECTION_SEPARATOR_WIDTH_60, '=') << "\n";
    std::cout << "Visual comparison for " << country << " (" << startYear << "-" << endYear << ")\n";
    std::cout << "Showing up to " << Constants::VERTICAL_CHART_MAX_PERIODS << " periods for optimal readability\n\n";
}

void PredictionChart::displayChart(const std::vector<Candlestick>& data, 
                                 const std::vector<std::vector<PredictionResult>>& allPredictions,
                                 TimeFrame timeframe, int startYear) {
    
    // Calculate temperature range
    double minTemp, maxTemp;
    calculateTemperatureRange(data, allPredictions, minTemp, maxTemp);
    
    const int chartHeight = Constants::VERTICAL_CHART_HEIGHT;
    const size_t maxPeriods = std::min(data.size(), 
                                      static_cast<size_t>(Constants::VERTICAL_CHART_MAX_PERIODS));
    double tempPerRow = (maxTemp - minTemp) / (chartHeight - 1);
    
    // Display temperature scale
    std::cout << "Temperature range: " << std::fixed << std::setprecision(Constants::TEMPERATURE_DISPLAY_PRECISION)
              << minTemp << "°C to " << maxTemp << "°C\n\n";
    
    // Display vertical chart
    for (int row = 0; row < chartHeight; row++) {
        double currentTemp = maxTemp - (row * tempPerRow);
        
        std::cout << std::right << std::setw(Constants::PREDICTION_VALUE_WIDTH) 
                  << std::fixed << std::setprecision(Constants::TEMPERATURE_DISPLAY_PRECISION) 
                  << currentTemp << " |";
        
        // Show data points for each period (limited by maxPeriods)
        for (size_t i = 0; i < maxPeriods; i++) {
            char symbol = getSymbolAtPosition(data, allPredictions, i, currentTemp, tempPerRow);
            std::cout << symbol;
        }
        std::cout << "\n";
    }
    
    // Display period labels
    std::cout << "       +";
    for (size_t i = 0; i < maxPeriods; i++) {
        std::cout << (i % Constants::PERIOD_LABEL_INTERVAL == 0 ? '|' : '-');
    }
    std::cout << "\n        ";
    
    // Period labels at intervals
    for (size_t i = 0; i < maxPeriods; i += Constants::PERIOD_LABEL_INTERVAL) {
        std::string label = formatPeriodLabel(i, timeframe, startYear);
        std::cout << std::left << std::setw(Constants::PERIOD_LABEL_INTERVAL) 
                  << label.substr(0, Constants::PERIOD_LABEL_MAX_LENGTH);
    }
    std::cout << "\n\n";
}

void PredictionChart::displayChartFooter() {
    std::cout << "LEGEND:\n";
    std::cout << "  o = Actual Temperature    ^ = Linear Regression\n";
    std::cout << "  # = Moving Average        + = Heuristic Model\n";
    std::cout << "  * = Prediction matches actual  x = Multiple predictions\n\n";
}

void PredictionChart::displayAccuracyAnalysis(const std::vector<Candlestick>& data,
                                             const std::vector<std::vector<PredictionResult>>& allPredictions) {
    
    std::cout << "=== PREDICTION ACCURACY ANALYSIS ===\n";
    
    // Calculate errors for each model
    std::vector<std::vector<double>> allErrors(3);
    
    // Linear model errors (index 0)
    if (allPredictions.size() > 0) {
        for (size_t i = Constants::MIN_LINEAR_DATA_SIZE; i < data.size(); i++) {
            if ((i - Constants::MIN_LINEAR_DATA_SIZE) < allPredictions[0].size() && 
                allPredictions[0][i - Constants::MIN_LINEAR_DATA_SIZE].isValid) {
                double actualTemp = data[i].getClose();
                double error = std::abs(actualTemp - allPredictions[0][i - Constants::MIN_LINEAR_DATA_SIZE].predictionValue);
                allErrors[0].push_back(error);
            }
        }
    }
    
    // Moving average errors (index 1)
    if (allPredictions.size() > 1) {
        for (size_t i = Constants::MIN_MOVING_AVERAGE_DATA_SIZE; i < data.size(); i++) {
            if ((i - Constants::MIN_MOVING_AVERAGE_DATA_SIZE) < allPredictions[1].size() && 
                allPredictions[1][i - Constants::MIN_MOVING_AVERAGE_DATA_SIZE].isValid) {
                double actualTemp = data[i].getClose();
                double error = std::abs(actualTemp - allPredictions[1][i - Constants::MIN_MOVING_AVERAGE_DATA_SIZE].predictionValue);
                allErrors[1].push_back(error);
            }
        }
    }
    
    // Heuristic model errors (index 2)
    if (allPredictions.size() > 2) {
        for (size_t i = Constants::MIN_HEURISTIC_DATA_SIZE; i < data.size(); i++) {
            if ((i - Constants::MIN_HEURISTIC_DATA_SIZE) < allPredictions[2].size() && 
                allPredictions[2][i - Constants::MIN_HEURISTIC_DATA_SIZE].isValid) {
                double actualTemp = data[i].getClose();
                double error = std::abs(actualTemp - allPredictions[2][i - Constants::MIN_HEURISTIC_DATA_SIZE].predictionValue);
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
    std::cout << std::string(Constants::ANALYSIS_SEPARATOR_WIDTH_65, '-') << "\n";
    
    const std::vector<std::string> modelNames = {"Linear Regression", "Moving Average", "Heuristic Model"};
    
    for (size_t i = 0; i < allErrors.size() && i < modelNames.size(); i++) {
        if (!allErrors[i].empty()) {
            double avgError = calculateMean(allErrors[i]);
            double maxError = *std::max_element(allErrors[i].begin(), allErrors[i].end());
            std::cout << std::left << std::setw(20) << modelNames[i]
                      << std::setw(15) << formatDouble(avgError, Constants::TEMPERATURE_DISPLAY_PRECISION + 1)
                      << std::setw(15) << formatDouble(maxError, Constants::TEMPERATURE_DISPLAY_PRECISION + 1)
                      << std::setw(15) << allErrors[i].size() << "\n";
        }
    }

    std::cout << "\nNote: This chart shows rolling predictions where each prediction uses only\n";
    std::cout << "data available up to that point, simulating real-world forecasting conditions.\n";
    std::cout << "Lower average error indicates better historical prediction accuracy.\n\n";
}

// Helper methods

std::vector<std::vector<PredictionResult>> PredictionChart::generateAllRollingPredictions(
    const std::vector<Candlestick>& data) {
    
    const std::vector<std::pair<std::function<PredictionResult(const std::vector<Candlestick>&)>, int>> models = {
        {[](const auto& d) { return Prediction::predictLinearWithConfidence(d); }, 
         static_cast<int>(Constants::MIN_LINEAR_DATA_SIZE)},
        {[](const auto& d) { return Prediction::predictMovingAverageWithConfidence(d, 3); }, 
         static_cast<int>(Constants::MIN_MOVING_AVERAGE_DATA_SIZE)},
        {[](const auto& d) { return Prediction::predictHeuristicWithConfidence(d); }, 
         static_cast<int>(Constants::MIN_HEURISTIC_DATA_SIZE)}
    };
    
    std::vector<std::vector<PredictionResult>> allPredictions;
    allPredictions.reserve(models.size());
    
    for (const auto& [modelFunc, startIdx] : models) {
        allPredictions.push_back(generateRollingPredictions(data, modelFunc, startIdx));
    }
    
    return allPredictions;
}

std::vector<PredictionResult> PredictionChart::generateRollingPredictions(
    const std::vector<Candlestick>& data,
    const std::function<PredictionResult(const std::vector<Candlestick>&)>& modelFunction,
    int startIndex) {
    
    std::vector<PredictionResult> predictions;
    predictions.reserve(data.size() - startIndex);
    
    for (size_t i = startIndex; i < data.size(); ++i) {
        const std::vector<Candlestick> trainingData(data.begin(), data.begin() + i);
        predictions.push_back(modelFunction(trainingData));
    }
    
    return predictions;
}

char PredictionChart::getSymbolAtPosition(const std::vector<Candlestick>& data,
                                         const std::vector<std::vector<PredictionResult>>& allPredictions,
                                         size_t periodIndex, double currentTemp, double tempPerRow) {
    
    double actualTemp = data[periodIndex].getClose();
    double tolerance = tempPerRow * Constants::TEMP_TOLERANCE_FACTOR;
    
    // Check if actual temperature is at this position
    bool actualAtPosition = (std::abs(actualTemp - currentTemp) <= tolerance);
    
    // Check predictions
    std::vector<bool> predictionsAtPosition(3, false);
    
    // Linear prediction (index 0)
    if (allPredictions.size() > 0 && periodIndex >= Constants::MIN_LINEAR_DATA_SIZE && 
        (periodIndex - Constants::MIN_LINEAR_DATA_SIZE) < allPredictions[0].size() && 
        allPredictions[0][periodIndex - Constants::MIN_LINEAR_DATA_SIZE].isValid) {
        
        double predValue = allPredictions[0][periodIndex - Constants::MIN_LINEAR_DATA_SIZE].predictionValue;
        predictionsAtPosition[0] = (std::abs(predValue - currentTemp) <= tolerance);
    }
    
    // Moving average prediction (index 1)
    if (allPredictions.size() > 1 && periodIndex >= Constants::MIN_MOVING_AVERAGE_DATA_SIZE && 
        (periodIndex - Constants::MIN_MOVING_AVERAGE_DATA_SIZE) < allPredictions[1].size() && 
        allPredictions[1][periodIndex - Constants::MIN_MOVING_AVERAGE_DATA_SIZE].isValid) {
        
        double predValue = allPredictions[1][periodIndex - Constants::MIN_MOVING_AVERAGE_DATA_SIZE].predictionValue;
        predictionsAtPosition[1] = (std::abs(predValue - currentTemp) <= tolerance);
    }
    
    // Heuristic prediction (index 2)
    if (allPredictions.size() > 2 && periodIndex >= Constants::MIN_HEURISTIC_DATA_SIZE && 
        (periodIndex - Constants::MIN_HEURISTIC_DATA_SIZE) < allPredictions[2].size() && 
        allPredictions[2][periodIndex - Constants::MIN_HEURISTIC_DATA_SIZE].isValid) {
        
        double predValue = allPredictions[2][periodIndex - Constants::MIN_HEURISTIC_DATA_SIZE].predictionValue;
        predictionsAtPosition[2] = (std::abs(predValue - currentTemp) <= tolerance);
    }
    
    // Determine symbol based on what's at this position
    int predictionCount = predictionsAtPosition[0] + predictionsAtPosition[1] + predictionsAtPosition[2];
    
    if (actualAtPosition && predictionCount > 0) {
        return '*'; // Actual and prediction(s) match
    } else if (actualAtPosition) {
        return 'o'; // Only actual temperature
    } else if (predictionCount > 1) {
        return 'x'; // Multiple predictions
    } else if (predictionsAtPosition[0]) {
        return '^'; // Linear prediction
    } else if (predictionsAtPosition[1]) {
        return '#'; // Moving average
    } else if (predictionsAtPosition[2]) {
        return '+'; // Heuristic prediction
    } else {
        return ' '; // Empty space
    }
}

void PredictionChart::calculateTemperatureRange(const std::vector<Candlestick>& data,
                                               const std::vector<std::vector<PredictionResult>>& allPredictions,
                                               double& minTemp, double& maxTemp) {
    
    minTemp = data[0].getClose();
    maxTemp = data[0].getClose();
    
    // Include actual temperatures
    for (const auto& candle : data) {
        minTemp = std::min(minTemp, candle.getClose());
        maxTemp = std::max(maxTemp, candle.getClose());
    }
    
    // Include prediction values
    for (const auto& predictions : allPredictions) {
        for (const auto& pred : predictions) {
            if (pred.isValid) {
                minTemp = std::min(minTemp, pred.predictionValue);
                maxTemp = std::max(maxTemp, pred.predictionValue);
            }
        }
    }
    
    // Apply padding using constants
    double range = maxTemp - minTemp;
    double padding = std::max(range * Constants::ENHANCED_CHART_PADDING_RATIO, 
                             Constants::MIN_CHART_PADDING);
    minTemp -= padding;
    maxTemp += padding;
}

std::string PredictionChart::formatPeriodLabel(size_t index, TimeFrame timeframe, int startYear) {
    switch (timeframe) {
        case TimeFrame::Yearly:
            return std::to_string(startYear + static_cast<int>(index));
            
        case TimeFrame::Monthly: {
            int year = startYear + static_cast<int>(index / 12);
            int month = static_cast<int>(index % 12) + 1;
            std::ostringstream oss;
            oss << year << "-" << std::setfill('0') << std::setw(2) << month;
            return oss.str();
        }
        
        case TimeFrame::Daily: {
            std::ostringstream oss;
            oss << "Day" << std::setfill('0') << std::setw(3) << (index + 1);
            return oss.str();
        }
        
        default:
            return "P" + std::to_string(index + 1);
    }
}

std::string PredictionChart::formatDouble(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}
// self written code end