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
    // Helper function to apply a chosen filter to a dataset
    std::vector<Candlestick> applyFilter(
        const std::vector<Candlestick>& data, // The source data to filter
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
                return data; // Return original data if filter type is invalid
        }
    }
}

int main() {
    const std::string filename = "weather_data_EU_1980-2019_temp_only.csv";
    
    try {
        // --- User Input & Data Loading ---
        std::string country = UserInput::getCountryCode();
        int startYear, endYear;
        UserInput::getYearRange(startYear, endYear);
        TimeFrame timeframe = UserInput::getTimeFrame();
        
        std::vector<TemperatureRecord> records = DataLoader::loadCSV(filename, country, startYear, endYear);
        std::cout << "Loaded " << records.size() << " records for " << country 
                  << " from " << startYear << " to " << endYear << ".\n";

        if (records.empty()) {
            std::cout << "Error: No data found for the specified criteria.\n";
            return 1;
        }

        // --- Candlestick Calculation ---
        const std::vector<Candlestick> candlesticks = CandlestickCalculator::computeCandlesticks(records, timeframe); // This is the original, immutable dataset
        std::cout << "Computed " << candlesticks.size() << " candlestick entries using " 
                  << Utils::timeFrameToString(timeframe) << " aggregation.\n";

        if (candlesticks.empty()) {
            std::cout << "Error: Failed to compute candlestick data from records.\n";
            return 1;
        }

        // --- Initial Plot of Full Dataset ---
        std::cout << "\n" << Utils::timeFrameToString(timeframe) << " candlestick chart for " 
                  << country << " (" << startYear << "-" << endYear << ") - Full Dataset:\n";
        Plotter::plotCandlesticks(candlesticks, timeframe, 20);

        // --- Filtering Loop ---
        std::vector<Candlestick> dataForAnalysis = candlesticks; // Initially, the analysis data is the full dataset
        int filtersApplied = 0;
        
        // Ask ONCE if the user wants to start filtering.
        if (UserInput::askForFiltering()) {
            while (true) { // Loop for applying multiple filters
                FilterType filterType;
                std::string startDate, endDate;
                double minTemp, maxTemp, minVolatility;
                bool uptrend;
                
                // Get criteria based on the ORIGINAL data to show full ranges
                if (UserInput::getFilterCriteria(candlesticks, timeframe, filterType, 
                                               startDate, endDate, minTemp, maxTemp, 
                                               uptrend, minVolatility)) {
                    
                    // Always apply the filter to the complete dataset
                    std::vector<Candlestick> filtered = applyFilter(
                        candlesticks, filterType, startDate, endDate, 
                        minTemp, maxTemp, uptrend, minVolatility
                    );
                    
                    if (!filtered.empty()) {
                        dataForAnalysis = filtered; // Update the data to be shown
                        filtersApplied++;
                        
                        std::cout << "\nFiltered " << Utils::timeFrameToString(timeframe) 
                                  << " candlestick chart for " << country 
                                  << " (Filter " << filtersApplied << " applied:\n";
                        Plotter::plotCandlesticks(dataForAnalysis, timeframe, 20);
                        
                    } else {
                        std::cout << "\nFilter resulted in no data. Showing previous view.\n";
                        Plotter::plotCandlesticks(dataForAnalysis, timeframe, 20);
                    }
                }

                // Ask if the user wants to apply ANOTHER filter. If not, exit the loop.
                if (!UserInput::askToContinue("apply another filter")) {
                    break;
                }
            }
        }
        
        // --- Final Summary ---
        std::cout << "\n=== Analysis Complete ===\n";
        std::cout << "Country: " << country << "\n";
        std::cout << "Timeframe: " << Utils::timeFrameToString(timeframe) << "\n";
        std::cout << "Original dataset: " << candlesticks.size() << " periods\n";
        std::cout << "Filters applied: " << filtersApplied << "\n";
        if (filtersApplied > 0) {
            std::cout << "Final dataset view: " << dataForAnalysis.size() << " periods\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "An unexpected error occurred." << std::endl;
        return 1;
    }

    return 0;
}