#include "Plotter.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cmath>

// struct holds all configurable aspects of the plot's appearance.
// It is defined here to keep it as an internal implementation detail of the Plotter.
struct PlotConfiguration {
    int candleWidth = 5;
    int candleSpacing = 2;
    std::string upTrendBody;
    std::string downTrendBody;
    std::string wickShape;
    std::string emptySpace;
    std::string spacingStr;

    // Generates the drawing strings based on the current width and spacing settings.
    void update() {
        upTrendBody = std::string(candleWidth, '+');
        downTrendBody = std::string(candleWidth, '-');
        emptySpace = std::string(candleWidth, ' ');
        spacingStr = std::string(candleSpacing, ' ');

        // Create a centered wick (e.g., "  |  " for width 5, " | " for width 3)
        if (candleWidth > 0) {
            wickShape = std::string(candleWidth, ' ');
            wickShape[candleWidth / 2] = '|';
        } else {
            wickShape = "";
        }
    }
};


namespace {
    // --- Chart Constants ---
    const int Y_AXIS_WIDTH = 8;       // Screen width for the Y-axis labels.
    const int MAX_CHART_WIDTH = 130;  // Max console width before compressing the plot.

    // ANSI escape codes for coloring console text.
    const std::string ANSI_COLOR_GREEN = "\033[32m";
    const std::string ANSI_COLOR_RED = "\033[31m";
    const std::string ANSI_COLOR_RESET = "\033[0m";
}

void Plotter::plotCandlesticks(const std::vector<Candlestick>& candlesticks, TimeFrame timeframe, int chartHeight) {
    if (candlesticks.empty()) {
        std::cout << "No candlestick data to plot.\n";
        return;
    }

    if (chartHeight <= 5) {
        chartHeight = 5;
    }

    // --- Automatic Plot Compression Logic ---
    PlotConfiguration config;
    int requiredWidth = candlesticks.size() * (config.candleWidth + config.candleSpacing) + Y_AXIS_WIDTH;
    bool compressed = false;

    // If the default width is too wide, try more compressed settings.
    if (requiredWidth > MAX_CHART_WIDTH) {
        compressed = true;
        config.candleWidth = 3; config.candleSpacing = 1; // Medium
        requiredWidth = candlesticks.size() * (config.candleWidth + config.candleSpacing) + Y_AXIS_WIDTH;
        if (requiredWidth > MAX_CHART_WIDTH) {
            config.candleWidth = 1; config.candleSpacing = 1; // High
            requiredWidth = candlesticks.size() * (config.candleWidth + config.candleSpacing) + Y_AXIS_WIDTH;
            if (requiredWidth > MAX_CHART_WIDTH) {
                config.candleWidth = 1; config.candleSpacing = 0; // Ultra
            }
        }
    }
    config.update(); // Finalize the plot style strings based on the chosen density.
    // --- End of Compression Logic ---


    double minTemp = findMinTemperature(candlesticks);
    double maxTemp = findMaxTemperature(candlesticks);
    
    double range = maxTemp - minTemp;
    minTemp -= range * 0.1;
    maxTemp += range * 0.1;
    range = maxTemp - minTemp;
    
    double tempPerRow = (range > 0) ? (range / (chartHeight - 1)) : 1.0;
    
    std::cout << "\n=== ASCII Candlestick Chart ===\n";
    if (compressed) {
        std::cout << "Note: Plot has been automatically compressed to fit the display.\n";
    }
    std::cout << "Legend: Wick: |, Up-Trend: " << ANSI_COLOR_GREEN << config.upTrendBody << ANSI_COLOR_RESET 
              << ", Down-Trend: " << ANSI_COLOR_RED << config.downTrendBody << ANSI_COLOR_RESET << "\n\n";
    
    for (int row = 0; row < chartHeight; ++row) {
        double currentTemp = maxTemp - (row * tempPerRow);
        std::cout << std::left << std::setw(Y_AXIS_WIDTH - 2) << std::fixed << std::setprecision(1) << currentTemp << "| ";
        
        for (const auto& candle : candlesticks) {
            std::cout << getCandlestickAtRow(candle, currentTemp, tempPerRow, config) << config.spacingStr;
        }
        std::cout << "\n";
    }
    
    const int totalChartWidth = candlesticks.size() * (config.candleWidth + config.candleSpacing);
    std::cout << std::string(Y_AXIS_WIDTH, '-') << std::string(totalChartWidth, '-') << "\n";
    
    std::cout << std::string(Y_AXIS_WIDTH, ' ');

    for (const auto& candle : candlesticks) {
        std::string label;
        const std::string& date = candle.getDate(); // YYYY-MM-DD
        
        // --- THIS SECTION IS UPDATED ---
        switch (timeframe) {
            case TimeFrame::Yearly:
                if (compressed) {
                    label = "'" + date.substr(2, 2); // 'YY format for compressed plots
                } else {
                    label = date.substr(0, 4);      // YYYY format for normal plots
                }
                break;
            case TimeFrame::Monthly:
                label = date.substr(5, 2) + "-" + date.substr(2, 2); // MM-YY
                break;
            case TimeFrame::Daily:
                label = date.substr(8, 2) + "-" + date.substr(5, 2); // DD-MM
                break;
        }
        // --- END OF UPDATED SECTION ---

        // Ensure label doesn't exceed the candle's width
        if(label.length() > config.candleWidth) {
            label = label.substr(0, config.candleWidth);
        }
        std::cout << std::setw(config.candleWidth) << std::left << label << config.spacingStr;
    }
    std::cout << "\n\n";
    
    printSummary(candlesticks);
}

std::string Plotter::getCandlestickAtRow(const Candlestick& candle, double rowTemp, double tempPerRow, const PlotConfiguration& config) {
    double high = candle.getHigh();
    double low = candle.getLow();
    double open = candle.getOpen();
    double close = candle.getClose();
    
    double bodyTop = std::max(open, close);
    double bodyBottom = std::min(open, close);

    double tolerance = tempPerRow / 2.0;

    bool inBody = (rowTemp <= bodyTop + tolerance) && (rowTemp >= bodyBottom - tolerance);
    bool inWick = (rowTemp <= high + tolerance) && (rowTemp >= low - tolerance);
    
    if (inBody) {
        bool isUpTrend = close >= open;
        return isUpTrend ? (ANSI_COLOR_GREEN + config.upTrendBody + ANSI_COLOR_RESET) 
                         : (ANSI_COLOR_RED + config.downTrendBody + ANSI_COLOR_RESET);
    } else if (inWick) {
        return config.wickShape;
    } else {
        return config.emptySpace;
    }
}

// Unchanged helper functions below...
double Plotter::findMinTemperature(const std::vector<Candlestick>& candlesticks) {
    if (candlesticks.empty()) return 0.0;
    double minTemp = candlesticks[0].getLow();
    for (const auto& candle : candlesticks) {
        if (candle.getLow() < minTemp) {
            minTemp = candle.getLow();
        }
    }
    return minTemp;
}

double Plotter::findMaxTemperature(const std::vector<Candlestick>& candlesticks) {
    if (candlesticks.empty()) return 0.0;
    double maxTemp = candlesticks[0].getHigh();
    for (const auto& candle : candlesticks) {
        if (candle.getHigh() > maxTemp) {
            maxTemp = candle.getHigh();
        }
    }
    return maxTemp;
}

void Plotter::printSummary(const std::vector<Candlestick>& candlesticks) {
    if (candlesticks.empty()) return;
    
    int upCount = 0;
    for (const auto& candle : candlesticks) {
        if (candle.getClose() >= candle.getOpen()) {
            upCount++;
        }
    }
    int downCount = candlesticks.size() - upCount;
    
    double upPercent = 100.0 * upCount / candlesticks.size();
    double downPercent = 100.0 * downCount / candlesticks.size();

    std::cout << "Summary:\n";
    std::cout << "  Total Periods: " << candlesticks.size() << "\n";
    std::cout << "  Up-Trends:     " << upCount << " (" << std::fixed << std::setprecision(1) << upPercent << "%)\n";
    std::cout << "  Down-Trends:   " << downCount << " (" << std::fixed << std::setprecision(1) << downPercent << "%)\n\n";
}