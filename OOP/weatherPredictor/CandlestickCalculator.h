#ifndef CANDLESTICK_CALCULATOR_H
#define CANDLESTICK_CALCULATOR_H

#include <vector>
#include <string>
#include "Candlestick.h"
#include "TemperatureRecord.h"
#include "Common.h"

namespace CandlestickCalculator {
    std::vector<Candlestick> computeCandlesticks(
        const std::vector<TemperatureRecord>& records, 
        TimeFrame timeframe
    );
    
    // Helper functions
    std::string getGroupKey(const std::string& dateTime, TimeFrame timeframe);
    std::string formatDateLabel(const std::string& groupKey, TimeFrame timeframe);
    void printCandlestickTable(const std::vector<Candlestick>& candlesticks);
}

#endif