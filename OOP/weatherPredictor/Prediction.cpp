#include "Prediction.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>

double Prediction::predictLinear(const std::vector<Candlestick>& data) {
    if (data.empty()) {
        return 0.0;
    }
    
    // Single data point - return that value
    if (data.size() == 1) {
        return data[0].getClose();
    }
    
    int n = data.size();
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    
    // Calculate the sums needed for the least squares formula
    // Using linear regression: y = mx + b
    for (int i = 0; i < n; i++) {
        double x = static_cast<double>(i);  // index as x-coordinate (time)
        double y = data[i].getClose();      // close temperature as y-coordinate
        
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }
    
    // Calculate the slope (m) using the formula:
    // m = (N*Σ(x*y) - Σx*Σy) / (N*Σ(x²) - (Σx)²)
    double denominator = n * sumX2 - sumX * sumX;
    
    if (std::abs(denominator) < 1e-10) {
        // If denominator is effectively 0, we have a degenerate case
        // (all x values same or perfectly horizontal line)
        // Return the average of y values
        return sumY / n;
    }
    
    double slope = (n * sumXY - sumX * sumY) / denominator;
    
    // Calculate the y-intercept (b) using the formula:
    // b = (Σy - m*Σx) / N
    double intercept = (sumY - slope * sumX) / n;
    
    // Predict the next value using the formula:
    // y = m*x + b where x is the next index (n)
    double nextX = static_cast<double>(n);
    double prediction = slope * nextX + intercept;
    
    return prediction;
}

double Prediction::predictMovingAverage(const std::vector<Candlestick>& data, int windowSize) {
    if (data.empty()) {
        return 0.0;
    }
    
    // Validate window size
    if (windowSize <= 0) {
        windowSize = 1;  // Default to at least 1
    }
    
    // Determine the actual window size (use the minimum of requested size and available data)
    int actualWindowSize = std::min(windowSize, static_cast<int>(data.size()));
    
    // Calculate the sum of the closing values for the last 'actualWindowSize' periods
    double sum = 0.0;
    int startIndex = data.size() - actualWindowSize;
    
    for (int i = startIndex; i < static_cast<int>(data.size()); i++) {
        sum += data[i].getClose();
    }
    
    // Return the average
    return sum / actualWindowSize;
}

double Prediction::predictHeuristic(const std::vector<Candlestick>& data) {
    if (data.empty()) {
        return 0.0;
    }
    
    // Single data point - no trend to extrapolate
    if (data.size() == 1) {
        return data[0].getClose();
    }
    
    // Get the last and second-to-last closing values
    double last = data.back().getClose();
    double secondLast = data[data.size() - 2].getClose();
    
    // Calculate the last change (momentum)
    double change = last - secondLast;
    
    // Predict that the same change will happen again
    // This is a momentum-based approach assuming recent trends continue
    double prediction = last + change;
    
    return prediction;
}

// Additional utility function to get prediction confidence/statistics
double Prediction::calculateLinearRSquared(const std::vector<Candlestick>& data) {
    if (data.size() < 2) {
        return 0.0;
    }
    
    int n = data.size();
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0, sumY2 = 0.0;
    
    // Calculate sums
    for (int i = 0; i < n; i++) {
        double x = static_cast<double>(i);
        double y = data[i].getClose();
        
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
        sumY2 += y * y;
    }
    
    // Calculate correlation coefficient
    double numerator = n * sumXY - sumX * sumY;
    double denominatorX = n * sumX2 - sumX * sumX;
    double denominatorY = n * sumY2 - sumY * sumY;
    
    if (denominatorX <= 0 || denominatorY <= 0) {
        return 0.0;
    }
    
    double correlation = numerator / std::sqrt(denominatorX * denominatorY);
    
    // R-squared is the square of the correlation coefficient
    return correlation * correlation;
}

// Helper function to calculate prediction accuracy metrics
std::vector<double> Prediction::calculateMovingAverageErrors(
    const std::vector<Candlestick>& data, int windowSize, int testPeriods) {
    
    std::vector<double> errors;
    
    if (data.size() < windowSize + testPeriods) {
        return errors;  // Not enough data
    }
    
    // Test the last 'testPeriods' predictions
    for (int i = 0; i < testPeriods; i++) {
        int endIndex = data.size() - testPeriods + i;
        
        // Create subset for prediction
        std::vector<Candlestick> subset(data.begin(), data.begin() + endIndex);
        
        // Make prediction
        double predicted = predictMovingAverage(subset, windowSize);
        double actual = data[endIndex].getClose();
        
        // Calculate absolute error
        double error = std::abs(predicted - actual);
        errors.push_back(error);
    }
    
    return errors;
}