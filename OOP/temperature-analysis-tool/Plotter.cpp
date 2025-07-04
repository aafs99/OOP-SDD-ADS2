#include "Plotter.h"
#include "PlotConfiguration.h"
#include "Common.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cmath>


/*self written code start (code built with reference to : 
 https://github.com/AriaFallah/csv-parser
 https://github.com/tdulcet/Table-and-Graph-Libs
 https://github.com/tdulcet/Table-and-Graph-Libs
*/
namespace {
    // ANSI color codes
    const std::string ANSI_COLOR_GREEN = "\033[32m";
    const std::string ANSI_COLOR_RED = "\033[31m";
    const std::string ANSI_COLOR_RESET = "\033[0m";

    // Compression levels
    const CompressionLevel COMPRESSION_LEVELS[] = {
        {5, 2, "Standard"},
        {3, 1, "Compact"},
        {2, 1, "Dense"},
        {1, 1, "Very Dense"},
        {1, 0, "Ultra Dense"}
    };
    const int NUM_COMPRESSION_LEVELS = sizeof(COMPRESSION_LEVELS) / sizeof(COMPRESSION_LEVELS[0]);

    // Use reference to avoid copying data unless necessary
    struct DisplayStrategy {
        std::vector<Candlestick> data;
        PlotConfiguration config;
        bool wasSampled;
        bool wasCompressed;
        std::string compressionLevel;
        
        // Move constructor to avoid unnecessary copies
        DisplayStrategy(std::vector<Candlestick>&& candlesticks) 
            : data(std::move(candlesticks)), wasSampled(false), wasCompressed(false), compressionLevel("Standard") {}
        
        DisplayStrategy() : wasSampled(false), wasCompressed(false), compressionLevel("Standard") {}
    };

    // Forward declarations for helper functions
    void printXAxisLabels(const std::vector<Candlestick>&, TimeFrame, const PlotConfiguration&);
    std::string formatDateLabel(const std::string& date, TimeFrame timeframe);
    
    /**
     *  Determines the optimal display strategy combining sampling and compression.
     */
    DisplayStrategy determineDisplayStrategy(const std::vector<Candlestick>& candlesticks, TimeFrame timeframe) {
        if (candlesticks.empty()) {
            DisplayStrategy strategy;
            strategy.config = PlotConfiguration(5, 2);
            return strategy;
        }
        
        size_t dataSize = candlesticks.size();
        size_t targetSize = dataSize;
        int startCompressionLevel = 0;
        bool needsSampling = false;
        
        // Determine if sampling or aggressive compression is needed based on data volume
        if (dataSize > Constants::MODERATE_MAX_DISPLAY_POINTS) {
            targetSize = Constants::ABSOLUTE_MAX_DISPLAY_POINTS;
            needsSampling = true;
            startCompressionLevel = 2; // Start with "Dense" for large datasets
        } else if (dataSize > Constants::PREFERRED_MAX_DISPLAY_POINTS) {
            startCompressionLevel = 1; // Start with "Compact" for medium datasets
        }
        
        // Only create strategy with data copy if sampling is needed
        if (needsSampling) {
            std::vector<Candlestick> sampled;
            sampled.reserve(targetSize);
            
            for (size_t i = 0; i < targetSize; ++i) {
                size_t index = (i * (dataSize - 1)) / (targetSize - 1);
                sampled.emplace_back(candlesticks[index]);
            }
            
            DisplayStrategy strategy(std::move(sampled));
            strategy.wasSampled = true;
            dataSize = strategy.data.size();
            
            // Find the best compression level that fits the screen width
            int finalCompressionLevel = startCompressionLevel;
            for (int i = startCompressionLevel; i < NUM_COMPRESSION_LEVELS; ++i) {
                finalCompressionLevel = i;
                const auto& level = COMPRESSION_LEVELS[i];
                int requiredWidth = dataSize * (level.candleWidth + level.candleSpacing) + Constants::Y_AXIS_WIDTH;
                if (requiredWidth <= Constants::MAX_CHART_WIDTH) {
                    break; // This level fits
                }
            }
            
            // Apply final configuration
            const auto& level = COMPRESSION_LEVELS[finalCompressionLevel];
            strategy.config = PlotConfiguration(level.candleWidth, level.candleSpacing);
            strategy.wasCompressed = (finalCompressionLevel > 0);
            strategy.compressionLevel = level.description;
            
            return strategy;
        } else {
            // No sampling needed, create strategy with reference to original data
            DisplayStrategy strategy;
            strategy.data.reserve(candlesticks.size());
            for (const auto& candle : candlesticks) {
                strategy.data.emplace_back(candle);
            }
            
            // Find the best compression level that fits the screen width
            int finalCompressionLevel = startCompressionLevel;
            for (int i = startCompressionLevel; i < NUM_COMPRESSION_LEVELS; ++i) {
                finalCompressionLevel = i;
                const auto& level = COMPRESSION_LEVELS[i];
                int requiredWidth = dataSize * (level.candleWidth + level.candleSpacing) + Constants::Y_AXIS_WIDTH;
                if (requiredWidth <= Constants::MAX_CHART_WIDTH) {
                    break; // This level fits
                }
            }
            
            // Apply final configuration
            const auto& level = COMPRESSION_LEVELS[finalCompressionLevel];
            strategy.config = PlotConfiguration(level.candleWidth, level.candleSpacing);
            strategy.wasCompressed = (finalCompressionLevel > 0);
            strategy.compressionLevel = level.description;
            
            return strategy;
        }
    }
    
    /**
     *  Prints an intelligent X-axis with labels at reasonable intervals.
     */
    void printXAxisLabels(const std::vector<Candlestick>& candlesticks, TimeFrame timeframe, const PlotConfiguration& config) {
        const int candleCharacterWidth = config.candleWidth + config.candleSpacing;
        if (candlesticks.empty() || candleCharacterWidth == 0) {
            std::cout << std::string(Constants::Y_AXIS_WIDTH, '-') << "\n\n";
            return;
        }

        const int totalChartWidth = candlesticks.size() * candleCharacterWidth;
        std::cout << std::string(Constants::Y_AXIS_WIDTH, '-') << std::string(totalChartWidth, '-') << "\n";
        std::cout << std::string(Constants::Y_AXIS_WIDTH, ' ');

        // Calculate a reasonable number of labels to display to avoid clutter
        int maxLabels = totalChartWidth / Constants::MIN_CHARS_PER_LABEL;
        maxLabels = std::max(2, maxLabels); // Ensure at least 2 labels (start and end)
        
        const int labelInterval = std::max(1, static_cast<int>(candlesticks.size() -1) / (maxLabels -1));
        
        for (size_t i = 0; i < candlesticks.size(); ++i) {
            // Determine if a label should be printed at this position
            bool isFirst = (i == 0);
            bool isLast = (i == candlesticks.size() - 1);
            bool atInterval = (labelInterval > 0 && i % labelInterval == 0);

            if (isFirst || isLast || atInterval) {
                std::string label = formatDateLabel(candlesticks[i].getDate(), timeframe);
                label = label.substr(0, candleCharacterWidth); // Truncate label to fit
                std::cout << std::left << std::setw(candleCharacterWidth) << label;
            } else {
                std::cout << std::string(candleCharacterWidth, ' '); // Print empty space
            }
        }

        std::cout << "\n\n";
    }
    
    /**
     *  Formats a date string into a readable label based on the timeframe.
     */
    std::string formatDateLabel(const std::string& date, TimeFrame timeframe) {
        if (date.length() < 10) return date;
        
        std::string year = date.substr(0, 4);
        std::string month = date.substr(5, 2);
        std::string day = date.substr(8, 2);
        std::string shortYear = date.substr(2, 2);
        
        const std::string monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        
        try {
            int monthNum = std::stoi(month);
            if (monthNum >= 1 && monthNum <= 12) {
                switch (timeframe) {
                    case TimeFrame::Daily:   
                        // Show actual day for daily timeframe
                        return day + "/" + month;  // e.g., "15/03" instead of "Mar'24"
                    case TimeFrame::Monthly: 
                        return monthNames[monthNum - 1] + " " + shortYear;
                    case TimeFrame::Yearly:  
                        return "'" + shortYear;
                }
            }
        } catch (...) { /* Fallback below */ }
        
        // Fallback to numeric format if something fails
        return date.substr(0, 7);
    }


    std::string getCandlestickAtRow(const Candlestick& candle, double rowTemp, double tempPerRow, const PlotConfiguration& config) {
        double high = candle.getHigh();
        double low = candle.getLow();
        double open = candle.getOpen();
        double close = candle.getClose();
        
        double bodyTop = std::max(open, close);
        double bodyBottom = std::min(open, close);
        double tolerance = tempPerRow / 2.0;

        
        if (rowTemp <= bodyTop + tolerance && rowTemp >= bodyBottom - tolerance) {
            return candle.isUptrend() ? (ANSI_COLOR_GREEN + config.upTrendBody + ANSI_COLOR_RESET) 
                                      : (ANSI_COLOR_RED + config.downTrendBody + ANSI_COLOR_RESET);
        }
        if (rowTemp <= high + tolerance && rowTemp >= low - tolerance) {
            return config.wickShape;
        }
        return config.emptySpace;
    }

    // Optimized to find both min and max in single pass
    std::pair<double, double> findTemperatureRange(const std::vector<Candlestick>& candlesticks) {
        if (candlesticks.empty()) return {0.0, 0.0};
        
        double minTemp = candlesticks[0].getLow();
        double maxTemp = candlesticks[0].getHigh();
        
        for (const auto& candle : candlesticks) {
            minTemp = std::min(minTemp, candle.getLow());
            maxTemp = std::max(maxTemp, candle.getHigh());
        }
        
        return {minTemp, maxTemp};
    }

    void printSummary(const std::vector<Candlestick>& candlesticks) {
        if (candlesticks.empty()) return;
        
        int upCount = 0;
        for (const auto& candle : candlesticks) {
            if (candle.isUptrend()) upCount++;
        }
        
        int downCount = candlesticks.size() - upCount;
        double upPercent = 100.0 * upCount / candlesticks.size();

        // Print summary
        std::cout << "Legend:\n Wick: |, Up-Trend: " << ANSI_COLOR_GREEN << PlotConfiguration().upTrendBody 
        << ANSI_COLOR_RESET << ", Down-Trend: " << ANSI_COLOR_RED << PlotConfiguration().downTrendBody 
        << ANSI_COLOR_RESET << "\n\n";
        
        std::cout << "Summary:\n";
        std::cout << "  Up-Trends:     " << upCount << " (" << std::fixed << std::setprecision(1) << upPercent << "%)\n";
        std::cout << "  Down-Trends:   " << downCount << " (" << std::fixed << std::setprecision(1) << (100.0 - upPercent) << "%)\n\n";
    }

} 

namespace Plotter {

void plotCandlesticks(const std::vector<Candlestick>& candlesticks, TimeFrame timeframe, int chartHeight) {
    if (candlesticks.empty()) {
        std::cout << "No candlestick data to plot.\n";
        return;
    }

    chartHeight = std::max(chartHeight, 5);

    DisplayStrategy strategy = determineDisplayStrategy(candlesticks, timeframe);
    
    // Use optimized single-pass temperature range finding
    std::pair<double, double> tempRange = findTemperatureRange(strategy.data);
    double minTemp = tempRange.first;
    double maxTemp = tempRange.second;
    
    double range = maxTemp - minTemp;
    if (range <= 0) range = 1.0;
    
    minTemp -= range * Constants::CHART_PADDING_RATIO;
    maxTemp += range * Constants::CHART_PADDING_RATIO;
    range = maxTemp - minTemp;
    
    double tempPerRow = (chartHeight > 1) ? (range / (chartHeight - 1)) : 0;
    
    for (int row = 0; row < chartHeight; ++row) {
        double currentTemp = maxTemp - (row * tempPerRow);
        
        std::cout << std::right << std::setw(Constants::Y_AXIS_WIDTH - 2) 
                  << std::fixed << std::setprecision(1) << currentTemp << "| ";
        
        for (const auto& candle : strategy.data) {
            std::cout << getCandlestickAtRow(candle, currentTemp, tempPerRow, strategy.config) 
                      << strategy.config.spacingStr;
        }
        std::cout << "\n";
    }
    
    printXAxisLabels(strategy.data, timeframe, strategy.config);
    printSummary(strategy.data);
}

} // namespace Plotter
//self written code end