#ifndef PLOTTER_H
#define PLOTTER_H

#include <vector>
#include <string>
#include "Candlestick.h"
#include "CandlestickCalculator.h" // Needed for the TimeFrame enum

// Forward-declare the internal configuration struct
struct PlotConfiguration;

/**
 * Plotter class to generate text-based candlestick charts.
 * Creates simple ASCII visualizations of temperature candlestick data.
 */
class Plotter {
public:
    /**
     * Outputs a text-based candlestick chart to the console.
     * This version automatically compresses the plot width for large datasets.
     * @param candlesticks The vector of Candlestick objects to visualize.
     * @param timeframe The time unit of the data (Yearly, Monthly, Daily) for correct axis labeling.
     * @param chartHeight The desired height of the chart in characters (rows).
     */
    static void plotCandlesticks(const std::vector<Candlestick>& candlesticks, TimeFrame timeframe, int chartHeight = 30);

private:
    /**
     * Gets the visual representation of a candlestick at a specific temperature row.
     * @param candle The candlestick to represent.
     * @param rowTemp The temperature level of the current row.
     * @param tempPerRow Temperature range represented by each row.
     * @param config The plot configuration object with current width and style settings.
     * @return String representation for this candlestick at this level, including color codes.
     */
    static std::string getCandlestickAtRow(const Candlestick& candle, double rowTemp, double tempPerRow, const PlotConfiguration& config);
    
    /**
     * Finds the minimum temperature across all candlesticks.
     * @param candlesticks Vector of candlestick data.
     * @return Minimum temperature found.
     */
    static double findMinTemperature(const std::vector<Candlestick>& candlesticks);
    
    /**
     * Finds the maximum temperature across all candlesticks.
     * @param candlesticks Vector of candlestick data.
     * @return Maximum temperature found.
     */
    static double findMaxTemperature(const std::vector<Candlestick>& candlesticks);
    
    /**
     * Prints summary statistics about the candlestick data.
     * @param candlesticks Vector of candlestick data.
     */
    static void printSummary(const std::vector<Candlestick>& candlesticks);
};

#endif