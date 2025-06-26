#include "Plotter.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cmath>

void Plotter::plotCandlesticks(const std::vector<Candlestick>& candlesticks) {
    if (candlesticks.empty()) {
        std::cout << "No candlestick data to plot.\n";
        return;
    }

    // Configuration
    const int CHART_HEIGHT = 15;
    const int CANDLESTICK_SPACING = 4;
    
    // Find temperature range
    double minTemp = findMinTemperature(candlesticks);
    double maxTemp = findMaxTemperature(candlesticks);
    
    // Add some padding
    double range = maxTemp - minTemp;
    minTemp -= range * 0.05;
    maxTemp += range * 0.05;
    range = maxTemp - minTemp;
    
    double tempPerRow = range / (CHART_HEIGHT - 1);
    
    std::cout << "\n=== ASCII Candlestick Chart ===\n";
    std::cout << "Legend:\n High/Low range: |, Up body: +++, Down body: --- = \n\n";
    
    // Print chart from top to bottom
    for (int row = 0; row < CHART_HEIGHT; ++row) {
        double currentTemp = maxTemp - (row * tempPerRow);
        
        // Print temperature label
        std::cout << std::fixed << std::setprecision(1) << std::setw(5) << currentTemp << " |";
        
        // Print candlesticks for this temperature level
        for (size_t i = 0; i < candlesticks.size(); ++i) {
            std::string candlestickRow = getCandlestickAtRow(candlesticks[i], currentTemp, tempPerRow);
            std::cout << candlestickRow;
            
            // Add spacing between candlesticks
            if (i < candlesticks.size() - 1) {
                std::cout << " ";
            }
        }
        std::cout << "\n";
    }
    
    // Print bottom border
    std::cout << "------";
    for (size_t i = 0; i < candlesticks.size(); ++i) {
        std::cout << "----";
        if (i < candlesticks.size() - 1) {
            std::cout << "-";
        }
    }
    std::cout << "\n";
    
    // Print date labels
    std::cout << "      ";
    for (size_t i = 0; i < candlesticks.size(); ++i) {
        std::string year = candlesticks[i].getDate().substr(0, 4);
        std::cout << std::setw(4) << year;
        if (i < candlesticks.size() - 1) {
            std::cout << " ";
        }
    }
    std::cout << "\n\n";
    
    // Print summary
    printSummary(candlesticks);
}

std::string Plotter::getCandlestickAtRow(const Candlestick& candle, double rowTemp, double tempPerRow) {
    double high = candle.getHigh();
    double low = candle.getLow();
    double open = candle.getOpen();
    double close = candle.getClose();
    
    // Check if this row intersects with the candlestick
    double tolerance = tempPerRow * 0.5;
    
    // Determine if we're in the wick area (between high and low)
    bool inWick = (rowTemp <= high + tolerance) && (rowTemp >= low - tolerance);
    
    // Determine if we're in the body area (between open and close)
    double bodyTop = std::max(open, close);
    double bodyBottom = std::min(open, close);
    bool inBody = (rowTemp <= bodyTop + tolerance) && (rowTemp >= bodyBottom - tolerance);
    
    if (!inWick) {
        return "   ";  // Empty space - 3 characters wide
    }
    
    if (inBody) {
        // In body area - show up or down trend
        bool isUp = close >= open;
        return isUp ? "+++" : "---";
    } else {
        // In wick area but not body - show vertical line
        return " | ";
    }
}

double Plotter::findMinTemperature(const std::vector<Candlestick>& candlesticks) {
    double minTemp = candlesticks[0].getLow();
    for (const auto& candle : candlesticks) {
        minTemp = std::min(minTemp, candle.getLow());
    }
    return minTemp;
}

double Plotter::findMaxTemperature(const std::vector<Candlestick>& candlesticks) {
    double maxTemp = candlesticks[0].getHigh();
    for (const auto& candle : candlesticks) {
        maxTemp = std::max(maxTemp, candle.getHigh());
    }
    return maxTemp;
}

void Plotter::printSummary(const std::vector<Candlestick>& candlesticks) {
    int upCount = 0;
    int downCount = 0;
    
    for (const auto& candle : candlesticks) {
        if (candle.getClose() >= candle.getOpen()) {
            upCount++;
        } else {
            downCount++;
        }
    }
    
    std::cout << "Summary:\n";
    std::cout << "  Total periods: " << candlesticks.size() << "\n";
    std::cout << "  Up trends: " << upCount << " (" 
              << std::fixed << std::setprecision(1) 
              << (100.0 * upCount / candlesticks.size()) << "%)\n";
    std::cout << "  Down trends: " << downCount << " (" 
              << std::fixed << std::setprecision(1) 
              << (100.0 * downCount / candlesticks.size()) << "%)\n";
    std::cout << std::endl;
}