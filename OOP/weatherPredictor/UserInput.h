#ifndef USER_INPUT_H
#define USER_INPUT_H

#include <string>
#include <vector>
#include "Common.h"

class Candlestick;

namespace UserInput {
    // Main input functions
    std::string getCountryCode();
    void getYearRange(int& startYear, int& endYear);
    TimeFrame getTimeFrame();
    bool askForFiltering();
    bool askForPredictions();
    bool askToContinue(const std::string& operation);
    
    // Filter-related functions
    bool getFilterCriteria(
        const std::vector<Candlestick>& candlesticks, 
        TimeFrame timeframe,
        FilterType& filterType, 
        std::string& startDate, 
        std::string& endDate,
        double& minTemp, 
        double& maxTemp, 
        bool& uptrend, 
        double& minVolatility
    );
    
    // Display functions
    void displayAvailableCountries();
    
    // Internal functions
    namespace Internal {
        void displayAvailableTimeFrames();
        bool getDateRangeFilter(
            const std::vector<Candlestick>& candlesticks, 
            TimeFrame timeframe,
            std::string& startDate, 
            std::string& endDate
        );
        bool getTemperatureRangeFilter(
            const std::vector<Candlestick>& candlesticks,
            double& minTemp, 
            double& maxTemp
        );
        bool getTrendFilter(bool& uptrend);
        bool getVolatilityFilter(
            const std::vector<Candlestick>& candlesticks, 
            double& minVolatility
        );
        
        std::string toUpperCase(const std::string& str);
        bool isValidCountryCode(const std::string& country);
        void clearInputBuffer();
        int getYear(const std::string& prompt, int minYear, int maxYear);
    }
}

#endif