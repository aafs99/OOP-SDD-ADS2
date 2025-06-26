#include <iostream>
#include <vector>
#include <iomanip>
#include <exception>
#include "Common.h"
#include "TemperatureRecord.h"
#include "DataLoader.h"
#include "CandlestickCalculator.h"
#include "Plotter.h"
#include "DataFilter.h"
#include "UserInput.h"

namespace {
    std::vector<Candlestick> applyFilter(
        const std::vector<Candlestick>& data,
        FilterType filterType,
        const std::string& startDate,
        const std::string& endDate,
        double minTemp,
        double maxTemp,
        bool uptrend,
        double minVolatility
    ) {
        switch (filterType) {
            case FilterType::DateRange:
                return DataFilter::filterByDateRange(data, startDate, endDate);
            case FilterType::TemperatureRange:
                return DataFilter::filterByTemperatureRange(data, minTemp, maxTemp);
            case FilterType::Trend:
                return DataFilter::filterByTrend(data, uptrend);
            case FilterType::Volatility:
                return DataFilter::filterByVolatility(data, minVolatility);
            default:
                std::cout << "Error: Invalid filter type.\n";
                return data;
        }
    }
}

int main() {
    const std::string filename = "weather_data_EU_1980-2019_temp_only.csv";
    
    try {
        // Get user input
        std::string country = UserInput::getCountryCode();
        int startYear, endYear;
        UserInput::getYearRange(startYear, endYear);
        TimeFrame timeframe = UserInput::getTimeFrame();
        
        // Load data
        std::vector<TemperatureRecord> records = DataLoader::loadCSV(filename, country, startYear, endYear);
        std::cout << "Loaded " << records.size() << " records for " << country 
                  << " from " << startYear << " to " << endYear << ".\n";

        if (records.empty()) {
            std::cout << "Error: No data found for the specified criteria.\n";
            return 1;
        }

        // Compute candlesticks
        std::vector<Candlestick> candlesticks = CandlestickCalculator::computeCandlesticks(records, timeframe);
        std::cout << "Computed " << candlesticks.size() << " candlestick entries using " 
                  << Utils::timeFrameToString(timeframe) << " aggregation.\n";

        if (candlesticks.empty()) {
            std::cout << "Error: Failed to compute candlestick data from records.\n";
            return 1;
        }

        // Plot full dataset
        std::cout << "\n" << Utils::timeFrameToString(timeframe) << " candlestick chart for " 
                  << country << " (" << startYear << "-" << endYear << ") - Full Dataset:\n";
        Plotter::plotCandlesticks(candlesticks, timeframe, 20);

        // Apply filters if requested
        std::vector<Candlestick> dataForAnalysis = candlesticks;
        int filtersApplied = 0;
        
        while (UserInput::askForFiltering()) {
            FilterType filterType;
            std::string startDate, endDate;
            double minTemp, maxTemp, minVolatility;
            bool uptrend;
            
            if (UserInput::getFilterCriteria(dataForAnalysis, timeframe, filterType, 
                                           startDate, endDate, minTemp, maxTemp, 
                                           uptrend, minVolatility)) {
                
                std::vector<Candlestick> filtered = applyFilter(
                    dataForAnalysis, filterType, startDate, endDate, 
                    minTemp, maxTemp, uptrend, minVolatility
                );
                
                if (!filtered.empty() && filtered.size() < dataForAnalysis.size()) {
                    dataForAnalysis = filtered;
                    filtersApplied++;
                    
                    std::cout << "\nFiltered " << Utils::timeFrameToString(timeframe) 
                              << " candlestick chart for " << country 
                              << " (Filter " << filtersApplied << " applied):\n";
                    Plotter::plotCandlesticks(filtered, timeframe, 20);
                    
                    if (!UserInput::askToContinue("apply another filter")) {
                        break;
                    }
                } else if (filtered.empty()) {
                    std::cout << "\nFilter resulted in no data. Using previous dataset.\n";
                } else {
                    std::cout << "\nFilter did not reduce dataset. Using previous dataset.\n";
                }
            }
        }
        
        std::cout << "\n=== Analysis Complete ===\n";
        std::cout << "Country: " << country << "\n";
        std::cout << "Timeframe: " << Utils::timeFrameToString(timeframe) << "\n";
        std::cout << "Original dataset: " << candlesticks.size() << " periods\n";
        std::cout << "Filters applied: " << filtersApplied << "\n";
        std::cout << "Final dataset: " << dataForAnalysis.size() << " periods\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "An unexpected error occurred." << std::endl;
        return 1;
    }

    return 0;
}