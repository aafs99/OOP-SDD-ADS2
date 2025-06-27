#include "CandlestickCalculator.h"
#include "Common.h"
#include <algorithm>
#include <map>
#include <iostream>
#include <iomanip>

namespace CandlestickCalculator {

std::vector<Candlestick> computeCandlesticks(const std::vector<TemperatureRecord>& records, TimeFrame timeframe) {
    std::vector<Candlestick> candlesticks;
    if (records.empty()) {
        return candlesticks;
    }

    std::vector<TemperatureRecord> sortedRecords = records;
    std::sort(sortedRecords.begin(), sortedRecords.end(), 
              [](const TemperatureRecord& a, const TemperatureRecord& b) {
                  return a.date < b.date;
              });

    std::map<std::string, std::vector<double>> groupedData;
    
    for (const auto& record : sortedRecords) {
        std::string groupKey = getGroupKey(record.date, timeframe);
        groupedData[groupKey].push_back(record.temperature);
    }

    for (const auto& group : groupedData) {
        const std::string& dateKey = group.first;
        const std::vector<double>& temperatures = group.second;
        
        if (temperatures.empty()) continue;
        
        double sum = 0.0;
        double high = temperatures[0];
        double low = temperatures[0];
        
        for (double temp : temperatures) {
            sum += temp;
            if (temp > high) high = temp;
            if (temp < low) low = temp;
        }
        
        double close = sum / temperatures.size();  // Average temperature for the period
        
        // FIXED: Use first temperature reading as "open" for meteorological accuracy
        // This makes more sense for temperature data than using previous period's average
        double open = temperatures[0];  // First temperature reading of the period
        
        std::string candlestickDate = formatDateLabel(dateKey, timeframe);
        
        candlesticks.push_back(Candlestick(candlestickDate, open, close, high, low));
    }
    
    printCandlestickTable(candlesticks);
    
    return candlesticks;
}

std::string getGroupKey(const std::string& dateTime, TimeFrame timeframe) {
    if (dateTime.length() < 10) {
        return "";
    }
    
    std::string year = dateTime.substr(0, 4);
    std::string month = dateTime.substr(5, 2);
    std::string day = dateTime.substr(8, 2);
    
    switch (timeframe) {
        case TimeFrame::Yearly:
            return year;
        case TimeFrame::Monthly:
            return year + "-" + month;
        case TimeFrame::Daily:
            return year + "-" + month + "-" + day;
        default:
            return year;
    }
}

std::string formatDateLabel(const std::string& groupKey, TimeFrame timeframe) {
    switch (timeframe) {
        case TimeFrame::Yearly:
            return groupKey + "-01-01";
        case TimeFrame::Monthly:
            return groupKey + "-01";
        case TimeFrame::Daily:
            return groupKey;
        default:
            return groupKey + "-01-01";
    }
}

void printCandlestickTable(const std::vector<Candlestick>& candlesticks) {
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
        std::string change = candle.isUptrend() ? "UP" : "DOWN";
        
        std::cout << std::left << std::setw(12) << candle.getDate()
                  << "| " << std::setw(8) << std::fixed << std::setprecision(2) << candle.getOpen()
                  << "| " << std::setw(8) << std::fixed << std::setprecision(2) << candle.getClose()
                  << "| " << std::setw(8) << std::fixed << std::setprecision(2) << candle.getHigh()
                  << "| " << std::setw(8) << std::fixed << std::setprecision(2) << candle.getLow()
                  << "| " << std::setw(6) << change << "\n";
    }
    std::cout << "\n";
}

} // namespace CandlestickCalculator