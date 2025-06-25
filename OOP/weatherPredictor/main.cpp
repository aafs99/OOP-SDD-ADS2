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
        
        // 1. Load raw temperature data for the specified country and year range
        std::vector<TemperatureRecord> records = DataLoader::loadCSV(filename, country, startYear, endYear);
        std::cout << "Loaded " << records.size() << " records for " << country 
                  << " from " << startYear << " to " << endYear << ".\n";

        if (records.empty()) {
            std::cout << "Warning: No data found for the specified criteria.\n";
            std::cout << "Please check:\n";
            std::cout << "  - CSV file exists and is accessible\n";
            std::cout << "  - Country code " << country << " exists in the CSV\n";
            std::cout << "  - Date range " << startYear << "-" << endYear << " contains data\n";
            return 1;
        }

        // 2. Compute candlestick data (default to yearly aggregation)
        std::vector<Candlestick> candlesticks = CandlestickCalculator::computeCandlesticks(records, TimeFrame::Yearly);
        std::cout << "Computed " << candlesticks.size() << " candlestick entries.\n";

        // 3. Plot the candlestick data as text output
        std::cout << "\nCandlestick chart for " << country << " (" << startYear << "-" << endYear << "):\n";
        Plotter::plotCandlesticks(candlesticks);

        // 4. Demonstrate filtering
        if (endYear - startYear >= 10) {  // Only filter if we have enough years
            int filterStartYear = startYear + (endYear - startYear) / 4;  // Start at 25% point
            int filterEndYear = startYear + 3 * (endYear - startYear) / 4;  // End at 75% point
            
            std::string filterStart = std::to_string(filterStartYear) + "-01-01";
            std::string filterEnd = std::to_string(filterEndYear) + "-01-01";
            
            std::vector<Candlestick> filtered = DataFilter::filterByDateRange(candlesticks, filterStart, filterEnd);
            std::cout << "\nFiltered candlestick chart for " << country 
                      << " (" << filterStartYear << " to " << filterEndYear << "):\n";
            Plotter::plotCandlesticks(filtered);
        }

        // 5. Predict future temperature trend using different models
        if (!candlesticks.empty()) {
            std::cout << "\n=== Temperature Predictions ===\n";
            double predLin = Prediction::predictLinear(candlesticks);
            double predMA = Prediction::predictMovingAverage(candlesticks, 3);
            double predHeur = Prediction::predictHeuristic(candlesticks);
            
            std::cout << "Predicted next period average temperature for " << country << ":\n";
            std::cout << "  Linear Regression model: " << predLin << "°C\n";
            std::cout << "  Moving Average model: " << predMA << "°C\n";
            std::cout << "  Heuristic model: " << predHeur << "°C\n";
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