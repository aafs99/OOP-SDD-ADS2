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
#include "Prediction.h"

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

    // Helper function to display prediction analysis
    void displayPredictions(const std::vector<Candlestick>& data, TimeFrame timeframe, 
                          const std::string& country, const std::string& dataDescription,
                          int startYear, int endYear) {
        if (data.empty()) {
            std::cout << "Cannot generate predictions: No data available.\n";
            return;
        }

        // prediction display
        Prediction::displayPredictionResults(data, timeframe, country, startYear, endYear);
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
        std::cout << "\n ==== " << Utils::timeFrameToString(timeframe) << " Candlestick Chart for " 
                  << country << " (" << startYear << "-" << endYear << ") ====\n\n";
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
                                  << " ==== Candlestick Chart for " << country 
                                  << " (Filter " << filtersApplied << " applied: ===\n";
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
        
        // --- Prediction Analysis ---
        if (UserInput::askForPredictions()) {
            std::string dataDescription = "complete dataset";
            if (filtersApplied > 0) {
                dataDescription = "filtered dataset (" + std::to_string(filtersApplied) + " filter" + 
                                (filtersApplied > 1 ? "s" : "") + " applied)";
            }
            
            displayPredictions(dataForAnalysis, timeframe, country, dataDescription, startYear, endYear);
            
            // Option to see predictions for original data if filters were applied
            if (filtersApplied > 0 && dataForAnalysis.size() != candlesticks.size()) {
                if (UserInput::askToContinue("see predictions for the complete (unfiltered) dataset")) {
                    displayPredictions(candlesticks, timeframe, country, "complete dataset", startYear, endYear);
                }
            }
        }
        
        // --- Final Summary ---
        std::cout << "\n=== Analysis Complete ===\n";        
        std::cout << "\nThank you for using the Temperature Analysis Tool!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "An unexpected error occurred." << std::endl;
        return 1;
    }

    return 0;
}