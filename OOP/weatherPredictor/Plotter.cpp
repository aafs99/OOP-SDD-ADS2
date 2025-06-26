#include "Plotter.h"
#include "PlotConfiguration.h"
#include "Common.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cmath>

namespace {
    // Chart display constants
    const int Y_AXIS_WIDTH = 8;
    const int MAX_CHART_WIDTH = 120;
    const double PADDING_RATIO = 0.1;

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

    // Sampling thresholds
    const size_t MAX_CANDLESTICKS_DAILY = 200;
    const size_t MAX_CANDLESTICKS_MONTHLY = 120;
    const size_t MAX_CANDLESTICKS_YEARLY = 80;

    // Helper function declarations (forward declaration not strictly needed here, but good practice)
    std::vector<Candlestick> applyIntelligentSampling(const std::vector<Candlestick>&, TimeFrame);
    PlotConfiguration determineOptimalCompression(size_t, TimeFrame);
    void printChartHeader(const PlotConfiguration&, bool, size_t, size_t);
    void printXAxisLabels(const std::vector<Candlestick>&, TimeFrame, const PlotConfiguration&);
    std::string getCandlestickAtRow(const Candlestick&, double, double, const PlotConfiguration&);
    double findMinTemperature(const std::vector<Candlestick>&);
    double findMaxTemperature(const std::vector<Candlestick>&);
    void printSummary(const std::vector<Candlestick>&);

    // Helper function implementations
    std::vector<Candlestick> applyIntelligentSampling(const std::vector<Candlestick>& candlesticks, TimeFrame timeframe) {
        if (candlesticks.empty()) {
            return candlesticks;
        }
        
        size_t maxCandlesticks;
        
        switch (timeframe) {
            case TimeFrame::Daily:   maxCandlesticks = MAX_CANDLESTICKS_DAILY;   break;
            case TimeFrame::Monthly: maxCandlesticks = MAX_CANDLESTICKS_MONTHLY; break;
            case TimeFrame::Yearly:  maxCandlesticks = MAX_CANDLESTICKS_YEARLY;  break;
            default:                 maxCandlesticks = MAX_CANDLESTICKS_YEARLY;  break;
        }
        
        if (candlesticks.size() <= maxCandlesticks) {
            return candlesticks;
        }
        
        std::vector<Candlestick> sampled;
        sampled.reserve(maxCandlesticks);
        
        for (size_t i = 0; i < maxCandlesticks && i < candlesticks.size(); ++i) {
            size_t index = (i * candlesticks.size()) / maxCandlesticks;
            if (index < candlesticks.size()) {
                sampled.push_back(candlesticks[index]);
            }
        }
        
        return sampled;
    }

    PlotConfiguration determineOptimalCompression(size_t numCandlesticks, TimeFrame timeframe) {
        for (int i = 0; i < NUM_COMPRESSION_LEVELS; ++i) {
            const auto& level = COMPRESSION_LEVELS[i];
            int requiredWidth = numCandlesticks * (level.candleWidth + level.candleSpacing) + Y_AXIS_WIDTH;
            
            if (requiredWidth <= MAX_CHART_WIDTH || i == NUM_COMPRESSION_LEVELS - 1) {
                return PlotConfiguration(level.candleWidth, level.candleSpacing);
            }
        }
        
        return PlotConfiguration(1, 0);
    }

    void printChartHeader(const PlotConfiguration& config, bool wasDownsampled, size_t originalSize, size_t displaySize) {
        std::cout << "\n=== ASCII Candlestick Chart ===\n";
        
        if (wasDownsampled) {
            std::cout << "Note: Showing " << displaySize << " of " << originalSize 
                      << " data points (sampled for display clarity)\n";
        }
        
        std::cout << "Legend: Wick: |, Up-Trend: " << ANSI_COLOR_GREEN << config.upTrendBody 
                  << ANSI_COLOR_RESET << ", Down-Trend: " << ANSI_COLOR_RED << config.downTrendBody 
                  << ANSI_COLOR_RESET << "\n\n";
    }

    std::string formatDateLabel(const std::string& date, TimeFrame timeframe, int maxWidth) {
        if (date.length() < 10) return date;
        
        std::string year = date.substr(0, 4);
        std::string month = date.substr(5, 2);
        std::string day = date.substr(8, 2);
        std::string label;
        
        switch (timeframe) {
            case TimeFrame::Yearly:  label = (maxWidth < 4) ? "'" + year.substr(2, 2) : year; break;
            case TimeFrame::Monthly: label = month + "/" + year.substr(2, 2); break;
            case TimeFrame::Daily:   label = day + "/" + month; break;
            default:                 label = year; break;
        }
        
        return (label.length() > static_cast<size_t>(maxWidth)) ? label.substr(0, maxWidth) : label;
    }

    void printXAxisLabels(const std::vector<Candlestick>& candlesticks, TimeFrame timeframe, const PlotConfiguration& config) {
        const int totalChartWidth = candlesticks.size() * (config.candleWidth + config.candleSpacing);
        
        std::cout << std::string(Y_AXIS_WIDTH, '-') << std::string(totalChartWidth, '-') << "\n";
        
        std::cout << std::string(Y_AXIS_WIDTH, ' ');
        for (const auto& candle : candlesticks) {
            std::string label = formatDateLabel(candle.getDate(), timeframe, config.candleWidth);
            std::cout << std::setw(config.candleWidth) << std::left << label << config.spacingStr;
        }
        std::cout << "\n\n";
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

    double findMinTemperature(const std::vector<Candlestick>& candlesticks) {
        if (candlesticks.empty()) return 0.0;
        
        double minTemp = candlesticks[0].getLow();
        for (const auto& candle : candlesticks) {
            minTemp = std::min(minTemp, candle.getLow());
        }
        return minTemp;
    }

    double findMaxTemperature(const std::vector<Candlestick>& candlesticks) {
        if (candlesticks.empty()) return 0.0;
        
        double maxTemp = candlesticks[0].getHigh();
        for (const auto& candle : candlesticks) {
            maxTemp = std::max(maxTemp, candle.getHigh());
        }
        return maxTemp;
    }

    void printSummary(const std::vector<Candlestick>& candlesticks) {
        if (candlesticks.empty()) return;
        
        int upCount = 0;
        for (const auto& candle : candlesticks) {
            if (candle.isUptrend()) upCount++;
        }
        
        int downCount = candlesticks.size() - upCount;
        double upPercent = 100.0 * upCount / candlesticks.size();
        
        std::cout << "Summary:\n";
        std::cout << "  Total Periods: " << candlesticks.size() << "\n";
        std::cout << "  Up-Trends:     " << upCount << " (" << std::fixed << std::setprecision(1) << upPercent << "%)\n";
        std::cout << "  Down-Trends:   " << downCount << " (" << std::fixed << std::setprecision(1) << (100.0 - upPercent) << "%)\n\n";
    }

} // anonymous namespace

namespace Plotter {

void plotCandlesticks(const std::vector<Candlestick>& candlesticks, TimeFrame timeframe, int chartHeight) {
    if (candlesticks.empty()) {
        std::cout << "No candlestick data to plot.\n";
        return;
    }

    chartHeight = std::max(chartHeight, 5);

    // Call helper functions from the anonymous namespace
    std::vector<Candlestick> displayData = applyIntelligentSampling(candlesticks, timeframe);
    bool wasDownsampled = displayData.size() < candlesticks.size();

    PlotConfiguration config = determineOptimalCompression(displayData.size(), timeframe);

    double minTemp = findMinTemperature(displayData);
    double maxTemp = findMaxTemperature(displayData);
    
    double range = maxTemp - minTemp;
    if (range <= 0) range = 1.0;
    
    minTemp -= range * PADDING_RATIO;
    maxTemp += range * PADDING_RATIO;
    range = maxTemp - minTemp;
    
    double tempPerRow = range / (chartHeight - 1);
    
    printChartHeader(config, wasDownsampled, candlesticks.size(), displayData.size());
    
    for (int row = 0; row < chartHeight; ++row) {
        double currentTemp = maxTemp - (row * tempPerRow);
        
        std::cout << std::right << std::setw(Y_AXIS_WIDTH - 2) 
                  << std::fixed << std::setprecision(1) << currentTemp << "| ";
        
        for (const auto& candle : displayData) {
            std::cout << getCandlestickAtRow(candle, currentTemp, tempPerRow, config) 
                      << config.spacingStr;
        }
        std::cout << "\n";
    }
    
    printXAxisLabels(displayData, timeframe, config);
    printSummary(displayData);
}

} // namespace Plotter