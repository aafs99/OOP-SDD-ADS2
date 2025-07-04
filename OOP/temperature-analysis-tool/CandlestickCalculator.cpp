#include "CandlestickCalculator.h"
#include "Common.h"
#include <algorithm>
#include <map>
#include <iostream>
#include <iomanip>

/*self written code start (code built with reference to : 
 https://github.com/KC1922/StockMarketDisplay-CPP/tree/main/StockProjectCPP
 https://github.com/VivekThazhathattil/candlesticks-SFML
*/

namespace CandlestickCalculator {

/**
 * This function groups temperature records by the specified timeframe, calculates
 * the open, close, high, and low values for each group, and returns them as a
 * vector of Candlestick objects. 
 * 
 * @param records A vector of TemperatureRecord structs.
 * @param timeframe The time frame (Yearly, Monthly, Daily) to group data by.
 * @return A vector of Candlestick objects.
 */
std::vector<Candlestick> computeCandlesticks(const std::vector<TemperatureRecord>& records, TimeFrame timeframe) {
    std::vector<Candlestick> candlesticks;
    if (records.empty()) {
        return candlesticks;
    }

    std::vector<TemperatureRecord> sortedRecords;
    sortedRecords.reserve(records.size());
    for (const auto& record : records) {
        sortedRecords.emplace_back(record);
    }
    
    std::sort(sortedRecords.begin(), sortedRecords.end(), 
              [](const TemperatureRecord& a, const TemperatureRecord& b) {
                  return a.date < b.date;
              });

    // Group records by the specified timeframe - using ordered map to maintain chronological order
    std::map<std::string, std::vector<double>> groupedData;
    
    for (const auto& record : sortedRecords) {
        std::string groupKey = getGroupKey(record.date, timeframe);
        if (!groupKey.empty()) {
            auto& temperatures = groupedData[groupKey];
            temperatures.emplace_back(record.temperature);
        }
    }

    // Reserve space for candlesticks to avoid reallocations
    candlesticks.reserve(groupedData.size());

    // Track previous period's average for correct "open" calculation
    double previousPeriodAverage = 0.0;
    bool hasPreviousPeriod = false;

    // Process each group to create a candlestick
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
        
        // Calculate current period's average temperature
        double currentPeriodAverage = sum / temperatures.size();
        
        // Use previous period's average as "open"
        // For the first period, use the current period's average as both open and close
        double open = hasPreviousPeriod ? previousPeriodAverage : currentPeriodAverage;
        
        // Close is the average temperature for the current period
        double close = currentPeriodAverage;
        
        std::string candlestickDate = formatDateLabel(dateKey, timeframe);
        
        // Use emplace_back to construct in place
        candlesticks.emplace_back(std::move(candlestickDate), open, close, high, low);
        
        // Update for next iteration
        previousPeriodAverage = currentPeriodAverage;
        hasPreviousPeriod = true;
    }
    
    return candlesticks;
}

/**
 *  Generates a key for grouping records based on the timeframe.
 * @param dateTime A string representing the date and time (e.g., "YYYY-MM-DD HH:MM:SS").
 * @param timeframe The timeframe for grouping.
 * @return A string key (e.g., "YYYY" for Yearly, "YYYY-MM" for Monthly).
 */
std::string getGroupKey(const std::string& dateTime, TimeFrame timeframe) {
    if (dateTime.length() < 10) {
        return ""; // Invalid date format
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
            return year; // Default case
    }
}

/**
 *  Formats the group key into a standardized date label.
 * @param groupKey The key generated by getGroupKey.
 * @param timeframe The timeframe used for grouping.
 * @return A formatted date string (e.g., "YYYY-01-01").
 */
std::string formatDateLabel(const std::string& groupKey, TimeFrame timeframe) {
    switch (timeframe) {
        case TimeFrame::Yearly:
            return groupKey + "-01-01"; // Represents the start of the year
        case TimeFrame::Monthly:
            return groupKey + "-01"; // Represents the start of the month
        case TimeFrame::Daily:
            return groupKey;
        default:
            return groupKey + "-01-01"; // Default case
    }
}

/**
 *  Prints a formatted table of candlestick data to the console.
 * This is a display function and should be called separately from the
 * data computation logic.
 * @param candlesticks A vector of Candlestick objects to display.
 */
void printCandlestickTable(const std::vector<Candlestick>& candlesticks) {
    if (candlesticks.empty()) {
        std::cout << "No candlestick data to display.\n";
        return;
    }
    
    // Table Header
    std::cout << "\n=== Candlestick Data Table ===\n";
    std::cout << std::left << std::setw(Constants::DATE_COLUMN_WIDTH) << "Date"
              << "| " << std::setw(Constants::TEMP_COLUMN_WIDTH) << "Open"
              << "| " << std::setw(Constants::TEMP_COLUMN_WIDTH) << "Close"
              << "| " << std::setw(Constants::TEMP_COLUMN_WIDTH) << "High"
              << "| " << std::setw(Constants::TEMP_COLUMN_WIDTH) << "Low"
              << "| " << std::setw(Constants::CHANGE_COLUMN_WIDTH) << "Change" << "\n";
    std::cout << std::string(Constants::TABLE_SEPARATOR_WIDTH, '-') << "\n";
    
    // Table Body
    for (const auto& candle : candlesticks) {
        std::string change = candle.isUptrend() ? "UP" : "DOWN";
        
        std::cout << std::left << std::setw(Constants::DATE_COLUMN_WIDTH) << candle.getDate()
                  << "| " << std::setw(Constants::TEMP_COLUMN_WIDTH) << std::fixed << std::setprecision(2) << candle.getOpen()
                  << "| " << std::setw(Constants::TEMP_COLUMN_WIDTH) << std::fixed << std::setprecision(2) << candle.getClose()
                  << "| " << std::setw(Constants::TEMP_COLUMN_WIDTH) << std::fixed << std::setprecision(2) << candle.getHigh()
                  << "| " << std::setw(Constants::TEMP_COLUMN_WIDTH) << std::fixed << std::setprecision(2) << candle.getLow()
                  << "| " << std::setw(Constants::CHANGE_COLUMN_WIDTH) << change << "\n";
    }
    std::cout << "\n";
}

} // namespace CandlestickCalculator
// self written code end