#include "Plotter.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cmath>

// Configuration struct holds all customizable aspects of the plot's appearance
struct PlotConfiguration {
    int candleWidth;
    int candleSpacing;
    std::string upTrendBody;
    std::string downTrendBody;
    std::string wickShape;
    std::string emptySpace;
    std::string spacingStr;

    // Constructor with default values
    PlotConfiguration(int width = 5, int spacing = 2) 
        : candleWidth(width), candleSpacing(spacing) {
        update();
    }

    // Updates the drawing strings based on current width and spacing settings
    void update() {
        upTrendBody = std::string(candleWidth, '+');
        downTrendBody = std::string(candleWidth, '-');
        emptySpace = std::string(candleWidth, ' ');
        spacingStr = std::string(candleSpacing, ' ');

        // Create a centered wick character
        if (candleWidth > 0) {
            wickShape = std::string(candleWidth, ' ');
            wickShape[candleWidth / 2] = '|';
        } else {
            wickShape = "|";  // Fallback for invalid width
        }
    }
};

namespace {
    // Chart display constants
    const int Y_AXIS_WIDTH = 8;
    const int MAX_CHART_WIDTH = 120;
    const double PADDING_RATIO = 0.1;  // 10% padding above and below data range

    // ANSI color codes for console output
    const std::string ANSI_COLOR_GREEN = "\033[32m";
    const std::string ANSI_COLOR_RED = "\033[31m";
    const std::string ANSI_COLOR_RESET = "\033[0m";

    // Compression levels for different display densities
    struct CompressionLevel {
        int candleWidth;
        int candleSpacing;
        std::string description;
    };

    const CompressionLevel COMPRESSION_LEVELS[] = {
        {5, 2, "Standard"},
        {3, 1, "Compact"},
        {1, 1, "Dense"},
        {1, 0, "Ultra-dense"}
    };
    const int NUM_COMPRESSION_LEVELS = sizeof(COMPRESSION_LEVELS) / sizeof(COMPRESSION_LEVELS[0]);
}

void Plotter::plotCandlesticks(const std::vector<Candlestick>& candlesticks, TimeFrame timeframe, int chartHeight) {
    if (candlesticks.empty()) {
        std::cout << "No candlestick data to plot.\n";
        return;
    }

    // Ensure minimum chart height for readability
    chartHeight = std::max(chartHeight, 5);

    // Determine optimal compression level
    PlotConfiguration config = determineOptimalCompression(candlesticks.size());

    // Calculate temperature range for the chart
    double minTemp = findMinTemperature(candlesticks);
    double maxTemp = findMaxTemperature(candlesticks);
    
    // Add padding to the temperature range
    double range = maxTemp - minTemp;
    if (range <= 0) range = 1.0;  // Prevent division by zero
    
    minTemp -= range * PADDING_RATIO;
    maxTemp += range * PADDING_RATIO;
    range = maxTemp - minTemp;
    
    double tempPerRow = range / (chartHeight - 1);
    
    // Display chart header and legend
    printChartHeader(config);
    
    // Draw the chart row by row
    for (int row = 0; row < chartHeight; ++row) {
        double currentTemp = maxTemp - (row * tempPerRow);
        
        // Print Y-axis label
        std::cout << std::right << std::setw(Y_AXIS_WIDTH - 2) 
                  << std::fixed << std::setprecision(1) << currentTemp << "| ";
        
        // Print candlesticks for this temperature level
        for (const auto& candle : candlesticks) {
            std::cout << getCandlestickAtRow(candle, currentTemp, tempPerRow, config) 
                      << config.spacingStr;
        }
        std::cout << "\n";
    }
    
    // Draw bottom border and X-axis labels
    printXAxisLabels(candlesticks, timeframe, config);
    printSummary(candlesticks);
}

PlotConfiguration Plotter::determineOptimalCompression(size_t numCandlesticks) {
    // Try each compression level to find the best fit
    for (int i = 0; i < NUM_COMPRESSION_LEVELS; ++i) {
        const auto& level = COMPRESSION_LEVELS[i];
        int requiredWidth = numCandlesticks * (level.candleWidth + level.candleSpacing) + Y_AXIS_WIDTH;
        
        if (requiredWidth <= MAX_CHART_WIDTH || i == NUM_COMPRESSION_LEVELS - 1) {
            PlotConfiguration config(level.candleWidth, level.candleSpacing);
            return config;
        }
    }
    
    // Fallback (should never reach here)
    return PlotConfiguration(1, 0);
}

void Plotter::printChartHeader(const PlotConfiguration& config) {
    std::cout << "\n=== ASCII Candlestick Chart ===\n";
    std::cout << "Legend: Wick: |, Up-Trend: " << ANSI_COLOR_GREEN << config.upTrendBody 
              << ANSI_COLOR_RESET << ", Down-Trend: " << ANSI_COLOR_RED << config.downTrendBody 
              << ANSI_COLOR_RESET << "\n\n";
}

void Plotter::printXAxisLabels(const std::vector<Candlestick>& candlesticks, 
                               TimeFrame timeframe, const PlotConfiguration& config) {
    // Calculate total chart width for border
    const int totalChartWidth = candlesticks.size() * (config.candleWidth + config.candleSpacing);
    
    // Print horizontal border
    std::cout << std::string(Y_AXIS_WIDTH, '-') << std::string(totalChartWidth, '-') << "\n";
    
    // Print X-axis labels
    std::cout << std::string(Y_AXIS_WIDTH, ' ');
    for (const auto& candle : candlesticks) {
        std::string label = formatDateLabel(candle.getDate(), timeframe, config.candleWidth);
        std::cout << std::setw(config.candleWidth) << std::left << label << config.spacingStr;
    }
    std::cout << "\n\n";
}

std::string Plotter::formatDateLabel(const std::string& date, TimeFrame timeframe, int maxWidth) {
    // Extract date components from "YYYY-MM-DD" format
    if (date.length() < 10) return date;  // Invalid format fallback
    
    std::string year = date.substr(0, 4);
    std::string month = date.substr(5, 2);
    std::string day = date.substr(8, 2);
    std::string label;
    
    switch (timeframe) {
        case TimeFrame::Yearly:
            // Use 'YY format for narrow displays, YYYY for wider ones
            label = (maxWidth < 4) ? "'" + year.substr(2, 2) : year;
            break;
        case TimeFrame::Monthly:
            label = month + "/" + year.substr(2, 2);  // MM/YY format
            break;
        case TimeFrame::Daily:
            label = day + "/" + month;  // DD/MM format
            break;
        default:
            label = year;
            break;
    }
    
    // Truncate label if it exceeds maximum width
    if (label.length() > static_cast<size_t>(maxWidth)) {
        label = label.substr(0, maxWidth);
    }
    
    return label;
}

std::string Plotter::getCandlestickAtRow(const Candlestick& candle, double rowTemp, 
                                         double tempPerRow, const PlotConfiguration& config) {
    double high = candle.getHigh();
    double low = candle.getLow();
    double open = candle.getOpen();
    double close = candle.getClose();
    
    double bodyTop = std::max(open, close);
    double bodyBottom = std::min(open, close);
    double tolerance = tempPerRow / 2.0;

    // Check if current row intersects with candlestick body
    if (rowTemp <= bodyTop + tolerance && rowTemp >= bodyBottom - tolerance) {
        bool isUpTrend = close >= open;
        return isUpTrend ? (ANSI_COLOR_GREEN + config.upTrendBody + ANSI_COLOR_RESET) 
                         : (ANSI_COLOR_RED + config.downTrendBody + ANSI_COLOR_RESET);
    }
    // Check if current row intersects with wick (but not body)
    else if (rowTemp <= high + tolerance && rowTemp >= low - tolerance) {
        return config.wickShape;
    }
    // Empty space
    else {
        return config.emptySpace;
    }
}

double Plotter::findMinTemperature(const std::vector<Candlestick>& candlesticks) {
    if (candlesticks.empty()) return 0.0;
    
    double minTemp = candlesticks[0].getLow();
    for (const auto& candle : candlesticks) {
        minTemp = std::min(minTemp, candle.getLow());
    }
    return minTemp;
}

double Plotter::findMaxTemperature(const std::vector<Candlestick>& candlesticks) {
    if (candlesticks.empty()) return 0.0;
    
    double maxTemp = candlesticks[0].getHigh();
    for (const auto& candle : candlesticks) {
        maxTemp = std::max(maxTemp, candle.getHigh());
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
    std::cout << "  Up-Trends:     " << upCount << " (" 
              << std::fixed << std::setprecision(1) << upPercent << "%)\n";
    std::cout << "  Down-Trends:   " << downCount << " (" 
              << std::fixed << std::setprecision(1) << downPercent << "%)\n\n";
}