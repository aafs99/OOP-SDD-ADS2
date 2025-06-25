#include "CandlestickCalculator.h"
#include <algorithm>
#include <map>
#include <iostream>
#include <iomanip>

std::vector<Candlestick> CandlestickCalculator::computeCandlesticks(const std::vector<TemperatureRecord>& records, TimeFrame timeframe) {
    std::vector<Candlestick> candlesticks;
    if (records.empty()) {
        return candlesticks;
    }

    // 1. Sort records by date if not already sorted
    std::vector<TemperatureRecord> sortedRecords = records;
    std::sort(sortedRecords.begin(), sortedRecords.end(), 
              [](const TemperatureRecord& a, const TemperatureRecord& b) {
                  return a.date < b.date;
              });

    // 2. Group records by timeframe using a map
    std::map<std::string, std::vector<double>> groupedData;
    
    for (const auto& record : sortedRecords) {
        std::string groupKey = getGroupKey(record.date, timeframe);
        groupedData[groupKey].push_back(record.temperature);
    }

    // 3. Process each group to create candlesticks
    double previousClose = 0.0;  // Will be set to first period's close for subsequent periods
    bool isFirstPeriod = true;
    
    for (const auto& group : groupedData) {
        const std::string& dateKey = group.first;
        const std::vector<double>& temperatures = group.second;
        
        if (temperatures.empty()) continue;  // Skip empty groups
        
        // Calculate statistics for this period
        double sum = 0.0;
        double high = temperatures[0];
        double low = temperatures[0];
        
        for (double temp : temperatures) {
            sum += temp;
            if (temp > high) high = temp;
            if (temp < low) low = temp;
        }
        
        double close = sum / temperatures.size();  // Average temperature (Close)
        double open;
        
        // Handle Open value based on whether this is the first period
        if (isFirstPeriod) {
            open = close;  // For first period, Open = Close
            isFirstPeriod = false;
        } else {
            open = previousClose;  // Open = previous period's Close
        }
        
        // Create date label for candlestick
        std::string candlestickDate = formatDateLabel(dateKey, timeframe);
        
        // Create and add candlestick
        Candlestick candlestick(candlestickDate, open, close, high, low);
        candlesticks.push_back(candlestick);
        
        // Update previousClose for next iteration
        previousClose = close;
    }
    
    // Print table of data
    printCandlestickTable(candlesticks);
    
    return candlesticks;
}

std::string CandlestickCalculator::getGroupKey(const std::string& dateTime, TimeFrame timeframe) {
    // Extract date components from "YYYY-MM-DD HH:MM:SS" format
    if (dateTime.length() < 10) {
        return "";  // Invalid date format
    }
    
    std::string year = dateTime.substr(0, 4);
    std::string month = dateTime.substr(5, 2);
    std::string day = dateTime.substr(8, 2);
    
    switch (timeframe) {
        case TimeFrame::Yearly:
            return year;  // Group by year: "1980"
        case TimeFrame::Monthly:
            return year + "-" + month;  // Group by year-month: "1980-01"
        case TimeFrame::Daily:
            return year + "-" + month + "-" + day;  // Group by full date: "1980-01-01"
        default:
            return year;  // Default to yearly
    }
}

std::string CandlestickCalculator::formatDateLabel(const std::string& groupKey, TimeFrame timeframe) {
    switch (timeframe) {
        case TimeFrame::Yearly:
            return groupKey + "-01-01";  // "1980" -> "1980-01-01"
        case TimeFrame::Monthly:
            return groupKey + "-01";     // "1980-01" -> "1980-01-01"
        case TimeFrame::Daily:
            return groupKey;             // "1980-01-01" -> "1980-01-01"
        default:
            return groupKey + "-01-01";
    }
}

void CandlestickCalculator::printCandlestickTable(const std::vector<Candlestick>& candlesticks) {
    if (candlesticks.empty()) {
        std::cout << "No candlestick data to display.\n";
        return;
    }
    
    std::cout << "\n=== Candlestick Data Table ===\n";
    std::cout << std::left << std::setw(12) << "Date"
              << "| " << std::setw(8) << "Open"
              << "| " << std::setw(8) << "Close"
              << "| " << std::setw(8) << "High"
              << "| " << std::setw(8) << "Low"
              << "| " << std::setw(6) << "Change" << "\n";
    std::cout << std::string(60, '-') << "\n";
    
    for (const auto& candle : candlesticks) {
        std::string Change = (candle.getClose() >= candle.getOpen()) ? "UP" : "DOWN";
        
        std::cout << std::left << std::setw(12) << candle.getDate()
                  << "| " << std::setw(8) << std::fixed << std::setprecision(2) << candle.getOpen()
                  << "| " << std::setw(8) << std::fixed << std::setprecision(2) << candle.getClose()
                  << "| " << std::setw(8) << std::fixed << std::setprecision(2) << candle.getHigh()
                  << "| " << std::setw(8) << std::fixed << std::setprecision(2) << candle.getLow()
                  << "| " << std::setw(6) << Change << "\n";
    }
    std::cout << "\n";
}