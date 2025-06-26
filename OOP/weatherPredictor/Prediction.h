#ifndef PREDICTION_H
#define PREDICTION_H

#include <vector>
#include "Candlestick.h"

/**
 * Prediction class housing different models for forecasting temperature trends.
 * 
 * This class provides three main prediction algorithms:
 * 1. Linear Regression - Uses least squares to fit a trend line
 * 2. Moving Average - Averages recent values for prediction
 * 3. Heuristic Model - Projects recent momentum forward
 */
class Prediction {
public:
    /**
     * Predict the next period's average temperature using Linear Regression model.
     * 
     * Uses the least squares method to fit a line through historical data points
     * and extrapolates to predict the next value. Formula: y = mx + b
     * 
     * @param data Vector of candlestick data (chronologically ordered)
     * @return Predicted temperature for the next period
     */
    static double predictLinear(const std::vector<Candlestick>& data);

    /**
     * Predict the next period's average temperature using a Moving Average model.
     * 
     * Calculates the average of the last 'windowSize' closing temperatures.
     * Good for smoothing out short-term fluctuations.
     * 
     * @param data Vector of candlestick data (chronologically ordered)
     * @param windowSize Number of last periods to average (default 3)
     * @return Predicted temperature for the next period
     */
    static double predictMovingAverage(const std::vector<Candlestick>& data, int windowSize = 3);

    /**
     * Predict the next period's average temperature using a heuristic-based model.
     * 
     * Uses momentum-based prediction: assumes the next change will be similar
     * to the most recent change. Formula: prediction = last + (last - secondLast)
     * 
     * @param data Vector of candlestick data (chronologically ordered)
     * @return Predicted temperature for the next period
     */
    static double predictHeuristic(const std::vector<Candlestick>& data);

    /**
     * Calculate the R-squared value for linear regression fit quality.
     * 
     * R-squared indicates how well the linear model fits the data:
     * - 1.0 = perfect fit
     * - 0.0 = no linear relationship
     * 
     * @param data Vector of candlestick data
     * @return R-squared value (0.0 to 1.0)
     */
    static double calculateLinearRSquared(const std::vector<Candlestick>& data);

    /**
     * Calculate prediction errors for moving average model validation.
     * 
     * Tests the moving average model on the last 'testPeriods' of historical data
     * to evaluate prediction accuracy.
     * 
     * @param data Vector of candlestick data
     * @param windowSize Moving average window size
     * @param testPeriods Number of periods to test (default 5)
     * @return Vector of absolute errors for each test period
     */
    static std::vector<double> calculateMovingAverageErrors(
        const std::vector<Candlestick>& data, 
        int windowSize, 
        int testPeriods = 5
    );
};

/**
 * Utility functions for prediction analysis
 */
namespace PredictionUtils {
    /**
     * Calculate Mean Absolute Error from a vector of errors
     */
    inline double calculateMAE(const std::vector<double>& errors) {
        if (errors.empty()) return 0.0;
        
        double sum = 0.0;
        for (double error : errors) {
            sum += error;
        }
        return sum / errors.size();
    }
    
    /**
     * Calculate Root Mean Square Error from a vector of errors
     */
    inline double calculateRMSE(const std::vector<double>& errors) {
        if (errors.empty()) return 0.0;
        
        double sumSquared = 0.0;
        for (double error : errors) {
            sumSquared += error * error;
        }
        return std::sqrt(sumSquared / errors.size());
    }
}

#endif