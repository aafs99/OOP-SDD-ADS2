#include "Prediction.h"
#include "Common.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>

double Prediction::predictLinear(const std::vector<Candlestick>& data) {
    if (data.empty()) return 0.0;
    if (data.size() == 1) return data[0].getClose();
    
    int n = data.size();
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    
    // Calculate sums needed for least squares formula
    for (int i = 0; i < n; i++) {
        double x = static_cast<double>(i);  // Time index
        double y = data[i].getClose();      // Temperature
        
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }
    
    // Calculate slope and intercept
    double denominator = n * sumX2 - sumX * sumX;
    if (std::abs(denominator) < 1e-10) {
        return sumY / n;  // Fallback to average
    }
    
    double slope = (n * sumXY - sumX * sumY) / denominator;
    double intercept = (sumY - slope * sumX) / n;
    
    // Predict next time index
    double nextX = static_cast<double>(n);
    return slope * nextX + intercept;
}

double Prediction::predictMovingAverage(const std::vector<Candlestick>& data, int windowSize) {
    if (data.empty()) return 0.0;
    
    if (windowSize <= 0) windowSize = 3;  // Default window
    int actualWindowSize = std::min(windowSize, static_cast<int>(data.size()));
    
    // Sum the last windowSize periods
    double sum = 0.0;
    for (int i = data.size() - actualWindowSize; i < static_cast<int>(data.size()); i++) {
        sum += data[i].getClose();
    }
    
    return sum / actualWindowSize;
}

double Prediction::predictHeuristic(const std::vector<Candlestick>& data) {
    if (data.empty()) return 0.0;
    if (data.size() == 1) return data[0].getClose();
    
    // Get recent values
    double last = data.back().getClose();
    double secondLast = data[data.size() - 2].getClose();
    
    // Calculate momentum and project forward
    double change = last - secondLast;
    return last + change;
}

void Prediction::displayPredictionResults(const std::vector<Candlestick>& data,
                                        TimeFrame timeframe,
                                        const std::string& country,
                                        int startYear, int endYear) {
    if (data.empty()) {
        std::cout << "Cannot generate predictions: No data available.\n";
        return;
    }

    // Determine what the "next period" actually means
    std::string nextPeriod = determineNextPeriod(data, timeframe, endYear);
    
    std::cout << "\n=== Temperature Prediction Analysis ===\n";
    std::cout << "Country: " << country << "\n";
    std::cout << "Data Range: " << startYear << "-" << endYear << " (" 
              << Utils::timeFrameToString(timeframe) << " data)\n";
    std::cout << "Data Points: " << data.size() << " periods\n";
    std::cout << "Predicting: " << nextPeriod << "\n\n";

    // Generate predictions
    double linearPred = predictLinear(data);
    double movingAvg = predictMovingAverage(data, 3);
    double heuristic = predictHeuristic(data);
    double rSquared = calculateLinearRSquared(data);

    // Display results with clear method descriptions
    std::cout << "=== Prediction Methods and Results ===\n\n";

    // 1. Linear Regression
    std::cout << "1. LINEAR REGRESSION MODEL\n";
    std::cout << "   Result: " << std::fixed << std::setprecision(2) << linearPred << "°C\n";
    std::cout << "   Confidence (R²): " << std::fixed << std::setprecision(3) << rSquared;
    if (rSquared > 0.7) {
        std::cout << " (STRONG trend)\n\n";
    } else if (rSquared > 0.4) {
        std::cout << " (MODERATE trend)\n\n";
    } else {
        std::cout << " (WEAK trend)\n\n";
    }

    // 2. Moving Average
    std::cout << "2. MOVING AVERAGE MODEL (3-period)\n";
    std::cout << "   Result: " << std::fixed << std::setprecision(2) << movingAvg << "°C\n";
    
    // Show the actual calculation
    if (data.size() >= 3) {
        std::cout << "   Calculation details: (" 
                  << std::fixed << std::setprecision(1)
                  << data[data.size()-3].getClose() << " + "
                  << data[data.size()-2].getClose() << " + "
                  << data[data.size()-1].getClose() << ") / 3 = "
                  << movingAvg << "°C\n\n";
    } else {
        std::cout << "\n";
    }

    // 3. Heuristic (Momentum)
    std::cout << "3. HEURISTIC (MOMENTUM) MODEL\n";
    std::cout << "   Result: " << std::fixed << std::setprecision(2) << heuristic << "°C\n";
    
    // Show the actual calculation
    if (data.size() >= 2) {
        double current = data[data.size()-1].getClose();
        double previous = data[data.size()-2].getClose();
        std::cout << "   Calculation details: " 
                  << std::fixed << std::setprecision(1) << current 
                  << " + (" << current << " - " << previous << ") = "
                  << heuristic << "°C\n\n";
    } else {
        std::cout << "\n";
    }

    // Summary comparison
    std::cout << "=== Prediction Summary for " << nextPeriod << " ===\n";
    std::cout << std::left << std::setw(20) << "Method" 
              << std::setw(15) << "Prediction" 
              << std::setw(25) << "Best Used For" << "\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << std::left << std::setw(20) << "Linear Regression" 
              << std::setw(15) << (std::to_string(linearPred).substr(0,5) + "°C")
              << std::setw(25) << "Long-term climate trends\n";
    std::cout << std::left << std::setw(20) << "Moving Average" 
              << std::setw(15) << (std::to_string(movingAvg).substr(0,5) + "°C")
              << std::setw(25) << "Stable, conservative forecast\n";
    std::cout << std::left << std::setw(20) << "Heuristic Model" 
              << std::setw(15) << (std::to_string(heuristic).substr(0,5) + "°C")
              << std::setw(25) << "Recent momentum trends\n\n";

    // Analysis and recommendation
    std::cout << "=== Analysis and Recommendation ===\n";
    
    // Find range of predictions
    double minPred = std::min({linearPred, movingAvg, heuristic});
    double maxPred = std::max({linearPred, movingAvg, heuristic});
    double range = maxPred - minPred;
    
    std::cout << "Prediction range: " << std::fixed << std::setprecision(1) << range << "°C ";
    if (range > 2.0) {
        std::cout << "(High uncertainty - models disagree)\n";
    } else if (range > 1.0) {
        std::cout << "(Moderate uncertainty)\n"; 
    } else {
        std::cout << "(Low uncertainty - models agree)\n";
    }
    
    // Model-specific insights
    if (heuristic == maxPred) {
        std::cout << "• Heuristic predicts highest temperature (warming momentum)\n";
    } else if (heuristic == minPred) {
        std::cout << "• Heuristic predicts lowest temperature (cooling momentum)\n";
    }
    
    // Recommendation based on data quality
    std::cout << "\nRecommendation: ";
    if (rSquared > 0.7 && data.size() >= 10) {
        std::cout << "Use Linear Regression (strong historical trend detected)\n";
    } else if (data.size() >= 5 && range <= 1.5) {
        std::cout << "Use Moving Average (models converge, stable prediction)\n";
    } else {
        std::cout << "Consider multiple models (compare results for uncertainty)\n";
    }
    
    std::cout << "\n";
}

std::string Prediction::determineNextPeriod(const std::vector<Candlestick>& data, 
                                           TimeFrame timeframe, int endYear) {
    if (data.empty()) return "Unknown";
    
    // Get the last date in the dataset
    std::string lastDate = data.back().getDate();
    
    switch (timeframe) {
        case TimeFrame::Yearly:
            return std::to_string(endYear + 1);
            
        case TimeFrame::Monthly:
            // Extract year and month from last date (YYYY-MM-DD format)
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

double Prediction::calculateLinearRSquared(const std::vector<Candlestick>& data) {
    if (data.size() < 2) return 0.0;
    
    int n = data.size();
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0, sumY2 = 0.0;
    
    for (int i = 0; i < n; i++) {
        double x = static_cast<double>(i);
        double y = data[i].getClose();
        
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
        sumY2 += y * y;
    }
    
    double numerator = n * sumXY - sumX * sumY;
    double denominatorX = n * sumX2 - sumX * sumX;
    double denominatorY = n * sumY2 - sumY * sumY;
    
    if (denominatorX <= 0 || denominatorY <= 0) return 0.0;
    
    double correlation = numerator / std::sqrt(denominatorX * denominatorY);
    return correlation * correlation;
}