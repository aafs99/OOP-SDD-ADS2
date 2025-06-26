#ifndef PREDICTION_H
#define PREDICTION_H

#include <vector>
#include <string>
#include "Candlestick.h"
#include "Common.h"

class Prediction {
public:
    /**
     * Linear Regression Prediction
     * Uses least squares method to fit trend line and extrapolate
     */
    static double predictLinear(const std::vector<Candlestick>& data);

    /**
     * Moving Average Prediction  
     * Calculates average of recent temperature values
     */
    static double predictMovingAverage(const std::vector<Candlestick>& data, int windowSize = 3);

    /**
     * Heuristic (Momentum) Prediction
     * Projects recent temperature change forward
     */
    static double predictHeuristic(const std::vector<Candlestick>& data);

    /**
     * Display prediction results
     */
    static void displayPredictionResults(const std::vector<Candlestick>& data,
                                       TimeFrame timeframe,
                                       const std::string& country,
                                       int startYear, int endYear);

    /**
     * Calculate R-squared for linear regression confidence
     */
    static double calculateLinearRSquared(const std::vector<Candlestick>& data);

private:
    /**
     * Determine what the "next period" means based on timeframe
     */
    static std::string determineNextPeriod(const std::vector<Candlestick>& data,
                                         TimeFrame timeframe, int endYear);
    
    /**
     * Get month name from number (1-12)
     */
    static std::string getMonthName(int month);
};

#endif