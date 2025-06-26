#include "Plotter.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cmath>

namespace {
    // --- Chart Configuration ---
    const int Y_AXIS_WIDTH = 8;       // Screen width for the Y-axis labels.
    const int CANDLE_WIDTH = 5;       // Must be odd for a centered wick.
    const int CANDLE_SPACING = 2;     // Spaces between candles.
    const int MAX_CHART_WIDTH = 150;  // Max console width before warning the user.

    // ANSI escape codes for coloring console text.
    const std::string ANSI_COLOR_GREEN = "\033[32m";
    const std::string ANSI_COLOR_RED = "\033[31m";
    const std::string ANSI_COLOR_RESET = "\033[0m";

    /**
     * Helper to create a string for the candle's wick, ensuring it's centered.
     */
    std::string createWickString() {
        if (CANDLE_WIDTH <= 0) return "";
        std::string wick = std::string(CANDLE_WIDTH, ' ');
        wick[CANDLE_WIDTH / 2] = '|';
        return wick;
    }

    // Define the visual representation of candle parts.
    const std::string UP_TREND_BODY   = std::string(CANDLE_WIDTH, '+');
    const std::string DOWN_TREND_BODY = std::string(CANDLE_WIDTH, '-');
    const std::string WICK_SHAPE      = createWickString();
    const std::string EMPTY_SPACE     = std::string(CANDLE_WIDTH, ' ');
    const std::string SPACING_STR     = std::string(CANDLE_SPACING, ' ');
}

void Plotter::plotCandlesticks(const std::vector<Candlestick>& candlesticks, TimeFrame timeframe, int chartHeight) {
    if (candlesticks.empty()) {
        std::cout << "No candlestick data to plot.\n";
        return;
    }

    // Handle high-density plots to avoid unreadable text wrap.
    const int requiredWidth = candlesticks.size() * (CANDLE_WIDTH + CANDLE_SPACING) + Y_AXIS_WIDTH;
    if (requiredWidth > MAX_CHART_WIDTH) {
        std::cerr << "Warning: Chart is too wide to display (" << candlesticks.size() << " periods).\n"
                  << "Please filter for a smaller date range.\n";
        return;
    }
    
    // Use chartHeight parameter instead of a fixed constant.
    if (chartHeight <= 5) { // Enforce a minimum sensible height
        chartHeight = 5;
    }

    double minTemp = findMinTemperature(candlesticks);
    double maxTemp = findMaxTemperature(candlesticks);
    
    double range = maxTemp - minTemp;
    minTemp -= range * 0.1;
    maxTemp += range * 0.1;
    range = maxTemp - minTemp;
    
    double tempPerRow = (range > 0) ? (range / (chartHeight - 1)) : 1.0;
    
    std::cout << "\n=== ASCII Candlestick Chart ===\n";
    std::cout << "Legend: Wick: |, Up-Trend: " << ANSI_COLOR_GREEN << UP_TREND_BODY << ANSI_COLOR_RESET 
              << ", Down-Trend: " << ANSI_COLOR_RED << DOWN_TREND_BODY << ANSI_COLOR_RESET << "\n\n";
    
    for (int row = 0; row < chartHeight; ++row) {
        double currentTemp = maxTemp - (row * tempPerRow);
        std::cout << std::left << std::setw(Y_AXIS_WIDTH - 2) << std::fixed << std::setprecision(1) << currentTemp << "| ";
        
        for (const auto& candle : candlesticks) {
            std::cout << getCandlestickAtRow(candle, currentTemp, tempPerRow) << SPACING_STR;
        }
        std::cout << "\n";
    }
    
    const int totalChartWidth = candlesticks.size() * (CANDLE_WIDTH + CANDLE_SPACING);
    std::cout << std::string(Y_AXIS_WIDTH, '-') << std::string(totalChartWidth, '-') << "\n";
    
    std::cout << std::string(Y_AXIS_WIDTH, ' ');

    // Adaptive X-Axis for different timeframes.
    for (const auto& candle : candlesticks) {
        std::string label;
        const std::string& date = candle.getDate(); // YYYY-MM-DD
        switch (timeframe) {
            case TimeFrame::Yearly:
                label = date.substr(0, 4); // "YYYY"
                break;
            case TimeFrame::Monthly:
                label = date.substr(5, 2) + "-" + date.substr(2, 2); // "MM-YY"
                break;
            case TimeFrame::Daily:
                label = date.substr(8, 2) + "-" + date.substr(5, 2); // "DD-MM"
                break;
        }
        std::cout << std::setw(CANDLE_WIDTH) << std::left << label << SPACING_STR;
    }
    std::cout << "\n\n";
    
    printSummary(candlesticks);
}

std::string Plotter::getCandlestickAtRow(const Candlestick& candle, double rowTemp, double tempPerRow) {
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
        // Add color to distinguish trends.
        bool isUpTrend = close >= open;
        return isUpTrend ? (ANSI_COLOR_GREEN + UP_TREND_BODY + ANSI_COLOR_RESET) 
                         : (ANSI_COLOR_RED + DOWN_TREND_BODY + ANSI_COLOR_RESET);
    } else if (inWick) {
        return WICK_SHAPE;
    } else {
        return EMPTY_SPACE;
    }
}

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