#ifndef PLOTTER_H
#define PLOTTER_H

#include <vector>
#include <string>
#include "Candlestick.h"

/**
 * Plotter class to generate text-based candlestick charts.
 * Creates simple ASCII visualizations of temperature candlestick data.
 */
class Plotter {
public:
    /**
     * Outputs a text-based candlestick chart to the console.
     * Shows temperature scale on left, candlesticks in middle, dates at bottom.
     * 
     * Chart symbols:
     * - '|' represents the wick (high-low range)
     * - '+++' represents up-trend body (close >= open)
     * - '---' represents down-trend body (close < open)
     * 
     * @param candlesticks Vector of Candlestick objects to visualize
     */
    static void plotCandlesticks(const std::vector<Candlestick>& candlesticks);

private:
    /**
     * Gets the visual representation of a candlestick at a specific temperature row.
     * @param candle The candlestick to represent
     * @param rowTemp The temperature level of the current row
     * @param tempPerRow Temperature range represented by each row
     * @return String representation (3 characters) for this candlestick at this level
     */
    static std::string getCandlestickAtRow(const Candlestick& candle, double rowTemp, double tempPerRow);
    
    /**
     * Finds the minimum temperature across all candlesticks.
     * @param candlesticks Vector of candlestick data
     * @return Minimum temperature found
     */
    static double findMinTemperature(const std::vector<Candlestick>& candlesticks);
    
    /**
     * Finds the maximum temperature across all candlesticks.
     * @param candlesticks Vector of candlestick data
     * @return Maximum temperature found
     */
    static double findMaxTemperature(const std::vector<Candlestick>& candlesticks);
    
    /**
     * Prints summary statistics about the candlestick data.
     * @param candlesticks Vector of candlestick data
     */
    static void printSummary(const std::vector<Candlestick>& candlesticks);
};

#endif