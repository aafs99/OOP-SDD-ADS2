#ifndef CANDLESTICKCALCULATOR_H
#define CANDLESTICKCALCULATOR_H

#include <vector>
#include <string>
#include "Candlestick.h"
#include "DataLoader.h"

/**
 * Enum to represent the aggregation timeframe for candlesticks.
 * (Yearly, Monthly, or Daily candlesticks)
 */
enum class TimeFrame { Yearly, Monthly, Daily };

/**
 * CandlestickCalculator class to compute candlestick data from raw temperature records.
 */
class CandlestickCalculator {
public:
    /**
     * Compute candlestick data from temperature records, grouping by the specified timeframe.
     * Returns a vector of Candlestick objects for each period.
     */
    static std::vector<Candlestick> computeCandlesticks(const std::vector<TemperatureRecord>& records, TimeFrame timeframe);

private:
    /**
     * Generate a grouping key for the specified timeframe from a datetime string.
     * @param dateTime Date string in "YYYY-MM-DD HH:MM:SS" format
     * @param timeframe The timeframe for grouping (Yearly, Monthly, Daily)
     * @return Grouping key (e.g., "1980" for yearly, "1980-01" for monthly)
     */
    static std::string getGroupKey(const std::string& dateTime, TimeFrame timeframe);
    
    /**
     * Format the group key into a proper date label for candlestick display.
     * @param groupKey The grouping key (e.g., "1980", "1980-01")
     * @param timeframe The timeframe used for grouping
     * @return Formatted date label (e.g., "1980-01-01")
     */
    static std::string formatDateLabel(const std::string& groupKey, TimeFrame timeframe);
    
    /**
     * Print a tabular representation of candlestick data for verification.
     * @param candlesticks Vector of Candlestick objects to display
     */
    static void printCandlestickTable(const std::vector<Candlestick>& candlesticks);
};

#endif