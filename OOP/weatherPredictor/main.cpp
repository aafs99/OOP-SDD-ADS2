#include <iostream>
#include <vector>
#include "DataLoader.h"
#include "CandlestickCalculator.h"
#include "Plotter.h"
#include "DataFilter.h"
#include "Prediction.h"
#include "UserInput.h"

int main() {
    std::string filename = "weather_data_EU_1980-2019_temp_only.csv";
    std::string country;
    int startYear, endYear;
    
    try {
        // Get user input with validation
        country = UserInput::getCountryCode();
        UserInput::getYearRange(startYear, endYear);
        TimeFrame timeframe = UserInput::getTimeFrame();
        
        // 1. Load raw temperature data for the specified country and year range
        std::vector<TemperatureRecord> records = DataLoader::loadCSV(filename, country, startYear, endYear);
        std::cout << "Loaded " << records.size() << " records for " << country 
                  << " from " << startYear << " to " << endYear << ".\n";

        if (records.empty()) {
            std::cout << "Error: No data found for the specified criteria.\n";
            std::cout << "Please check:\n";
            std::cout << "  - CSV file exists and is accessible\n";
            std::cout << "  - Country code " << country << " exists in the CSV\n";
            std::cout << "  - Date range " << startYear << "-" << endYear << " contains data\n";
            return 1;
        }

        // 2. Compute candlestick data using user-selected timeframe
        std::vector<Candlestick> candlesticks = CandlestickCalculator::computeCandlesticks(records, timeframe);
        std::cout << "Computed " << candlesticks.size() << " candlestick entries using " 
                  << UserInput::timeFrameToString(timeframe) << " aggregation.\n";

        if (candlesticks.empty()) {
            std::cout << "Error: Failed to compute candlestick data from records.\n";
            return 1;
        }

        // 3. Plot the candlestick data as text output
        std::cout << "\n" << UserInput::timeFrameToString(timeframe) << " candlestick chart for " 
                  << country << " (" << startYear << "-" << endYear << "):\n";
        Plotter::plotCandlesticks(candlesticks, timeframe, 20);

        // 4. Demonstrate filtering (only if we have sufficient data)
        int minPeriodsForFiltering = (timeframe == TimeFrame::Daily) ? 30 : 
                                   (timeframe == TimeFrame::Monthly) ? 12 : 10;
        
        if (candlesticks.size() >= static_cast<size_t>(minPeriodsForFiltering)) {
            // Filter to middle 50% of the data (skip first and last 25%)
            size_t skipPeriods = candlesticks.size() / 4;
            size_t startIndex = skipPeriods;
            size_t endIndex = candlesticks.size() - skipPeriods - 1;
            
            if (startIndex < endIndex && endIndex < candlesticks.size()) {
                std::string filterStart = candlesticks[startIndex].getDate();
                std::string filterEnd = candlesticks[endIndex].getDate();
                
                std::vector<Candlestick> filtered = DataFilter::filterByDateRange(candlesticks, filterStart, filterEnd);
                
                if (!filtered.empty()) {
                    std::cout << "\nFiltered " << UserInput::timeFrameToString(timeframe) 
                              << " candlestick chart for " << country 
                              << " (middle 50% of data):\n";
                    Plotter::plotCandlesticks(filtered, timeframe, 15);
                }
            }
        } else {
            std::cout << "\nSkipping filtering demonstration - need at least " 
                      << minPeriodsForFiltering << " " << UserInput::timeFrameToString(timeframe) 
                      << " periods for meaningful filtering.\n";
        }

        // 5. Predict future temperature trend using different models
        std::cout << "\n=== Temperature Predictions ===\n";
        
        int minDataPointsForPrediction = (timeframe == TimeFrame::Daily) ? 7 : 
                                       (timeframe == TimeFrame::Monthly) ? 6 : 3;
        
        if (candlesticks.size() >= static_cast<size_t>(minDataPointsForPrediction)) {
            double predLin = Prediction::predictLinear(candlesticks);
            double predMA = Prediction::predictMovingAverage(candlesticks, 3);
            double predHeur = Prediction::predictHeuristic(candlesticks);
            
            std::cout << "Predicted next " << UserInput::timeFrameToString(timeframe) 
                      << " average temperature for " << country << ":\n";
                      std::cout << "  Linear Regression model: " << predLin << "°C\n";
                      std::cout << "  Moving Average model: " << predMA << "°C\n";
                      std::cout << "  Heuristic model: " << predHeur << "°C\n";        } else {
            std::cout << "Insufficient data for reliable predictions (need at least " 
                      << minDataPointsForPrediction << " " << UserInput::timeFrameToString(timeframe) 
                      << " periods).\n";
        }
        
        std::cout << "\n=== Analysis Complete ===\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "An unexpected error occurred." << std::endl;
        return 1;
    }

    return 0;
}