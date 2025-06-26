#ifndef PLOTTER_H
#define PLOTTER_H

#include <vector>
#include <string>
#include "Candlestick.h"
#include "CandlestickCalculator.h" // For TimeFrame enum

// Forward declaration
struct PlotConfiguration;

/**
 * Plotter class to generate text-based candlestick charts.
 * Creates ASCII visualizations of temperature candlestick data with automatic
 * compression for large datasets and intelligent formatting.
 */
class Plotter {
public:
    /**
     * Outputs a text-based candlestick chart to the console.
     * Automatically determines optimal compression and formatting based on data size.
     * 
     * @param candlesticks Vector of Candlestick objects to visualize
     * @param timeframe Time unit of the data for correct axis labeling
     * @param chartHeight Desired height of the chart in rows (minimum 5)
     */
    static void plotCandlesticks(const std::vector<Candlestick>& candlesticks, 
                                TimeFrame timeframe, int chartHeight = 20);

private:
    /**
     * Applies intelligent sampling to reduce the number of candlesticks for display.
     * Uses timeframe-appropriate limits to ensure readable charts.
     * 
     * @param candlesticks Original candlestick data
     * @param timeframe Current timeframe (affects sampling limits)
     * @return Sampled candlestick data for display
     */
    static std::vector<Candlestick> applyIntelligentSampling(const std::vector<Candlestick>& candlesticks, TimeFrame timeframe);

    /**
     * Determines the optimal compression level based on number of candlesticks and timeframe.
     * 
     * @param numCandlesticks Number of candlesticks to display
     * @param timeframe The timeframe affects compression aggressiveness
     * @return PlotConfiguration with optimal width and spacing settings
     */
    static PlotConfiguration determineOptimalCompression(size_t numCandlesticks, TimeFrame timeframe);

    /**
     * Prints the chart header with title, legend, and sampling information.
     * 
     * @param config Plot configuration containing display settings
     * @param wasDownsampled Whether data was sampled for display
     * @param originalSize Original number of data points
     * @param displaySize Number of data points being displayed
     */
    static void printChartHeader(const PlotConfiguration& config, bool wasDownsampled = false, 
                                size_t originalSize = 0, size_t displaySize = 0);

    /**
     * Prints the X-axis labels and bottom border of the chart.
     * 
     * @param candlesticks Vector of candlestick data
     * @param timeframe Time unit for label formatting
     * @param config Plot configuration settings
     */
    static void printXAxisLabels(const std::vector<Candlestick>& candlesticks, 
                                TimeFrame timeframe, const PlotConfiguration& config);

    /**
     * Formats a date string for display as an X-axis label.
     * 
     * @param date Date string in "YYYY-MM-DD" format
     * @param timeframe Time unit determining label format
     * @param maxWidth Maximum characters available for the label
     * @return Formatted date label
     */
    static std::string formatDateLabel(const std::string& date, TimeFrame timeframe, int maxWidth);

    /**
     * Gets the visual representation of a candlestick at a specific temperature row.
     * 
     * @param candle The candlestick to represent
     * @param rowTemp The temperature level of the current row
     * @param tempPerRow Temperature range represented by each row
     * @param config Plot configuration with current width and style settings
     * @return String representation for this candlestick at this level
     */
    static std::string getCandlestickAtRow(const Candlestick& candle, double rowTemp, 
                                          double tempPerRow, const PlotConfiguration& config);
    
    /**
     * Finds the minimum temperature across all candlesticks.
     * 
     * @param candlesticks Vector of candlestick data
     * @return Minimum temperature found
     */
    static double findMinTemperature(const std::vector<Candlestick>& candlesticks);
    
    /**
     * Finds the maximum temperature across all candlesticks.
     * 
     * @param candlesticks Vector of candlestick data
     * @return Maximum temperature found
     */
    static double findMaxTemperature(const std::vector<Candlestick>& candlesticks);
    
    /**
     * Prints summary statistics about the candlestick data.
     * 
     * @param candlesticks Vector of candlestick data
     */
    static void printSummary(const std::vector<Candlestick>& candlesticks);
};

#endif