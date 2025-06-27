#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>
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
    // Use move semantics to avoid unnecessary copies
    std::vector<Candlestick> applyFilter(
        std::vector<Candlestick>&& data, // Accept by rvalue reference
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
                return std::move(data); // Return original data if filter type is invalid
        }
    }

    // Helper function to display prediction analysis
    void displayPredictions(const std::vector<Candlestick>& data, TimeFrame timeframe, 
                          const std::string& country, int startYear, int endYear) {
        if (data.empty()) {
            std::cout << "Cannot generate predictions: No data available.\n";
            return;
        }

        // Use the simplified prediction display
        Prediction::displayPredictionResults(data, timeframe, country, startYear, endYear);
    }
}

int main() {
    const std::string filename = "./weather_data_EU_1980-2019_temp_only.csv";
    
    // Clear screen and show welcome message
    UserInput::clearScreen();
    std::cout << "=================================================================\n";
    std::cout << "    EUROPEAN TEMPERATURE ANALYSIS TOOL WITH PREDICTIONS\n";
    std::cout << "                    (1980-2019 Historical Data)\n";
    std::cout << "=================================================================\n";
    std::cout << "\nWelcome! This tool helps you analyze and predict temperature trends\n";
    std::cout << "across European countries using historical climate data.\n\n";
    
    UserInput::waitForUser();
    
    // Main analysis loop - allows analyzing multiple countries
    while (true) {
        try {
            UserInput::clearScreen();
            
            // --- User Input & Data Loading ---
            std::cout << "=== STARTING NEW ANALYSIS ===\n\n";
            
            std::string country = UserInput::getCountryCode();
            UserInput::clearScreen();
            
            int startYear, endYear;
            UserInput::getYearRange(startYear, endYear);
            UserInput::clearScreen();
            
            TimeFrame timeframe = UserInput::getTimeFrame();
            UserInput::clearScreen();
            
            std::cout << "=== LOADING DATA ===\n";
            std::cout << "Loading temperature data for " << country 
                      << " from " << startYear << " to " << endYear << "...\n";
            
            std::vector<TemperatureRecord> records = DataLoader::loadCSV(filename, country, startYear, endYear);
            std::cout << "✓ Loaded " << records.size() << " records for " << country 
                      << " from " << startYear << " to " << endYear << ".\n";

            if (records.empty()) {
                std::cout << "Error: No data found for the specified criteria.\n";
                std::cout << "Please try different parameters.\n";
                UserInput::waitForUser();
                continue; // Go back to country selection
            }

            // --- Candlestick Calculation ---
            std::cout << "\n=== PROCESSING DATA ===\n";
            std::cout << "Computing candlestick aggregation using " 
                      << Utils::timeFrameToString(timeframe) << " timeframe...\n";
            
            const std::vector<Candlestick> candlesticks = CandlestickCalculator::computeCandlesticks(records, timeframe);
            std::cout << "✓ Computed " << candlesticks.size() << " candlestick entries.\n";

            if (candlesticks.empty()) {
                std::cout << "Error: Failed to compute candlestick data from records.\n";
                UserInput::waitForUser();
                continue;
            }

            UserInput::waitForUser();
            UserInput::clearScreen();

            // --- Initial Plot of Full Dataset ---
            std::cout << "==== " << Utils::timeFrameToString(timeframe) << " Candlestick Chart for " 
                      << country << " (" << startYear << "-" << endYear << ") ====\n\n";
            Plotter::plotCandlesticks(candlesticks, timeframe, 20);

            // --- Show Candlestick Data Table ---
            if (UserInput::askToContinue("view the detailed candlestick data table")) {
                CandlestickCalculator::printCandlestickTable(candlesticks);
                UserInput::waitForUser();
            }

            // Filtering Loop with cumulative filtering option ---
            std::vector<Candlestick> dataForAnalysis = candlesticks;
            int filtersApplied = 0;
            
            if (UserInput::askForFiltering()) {
                UserInput::clearScreen();
                
                while (true) {
                    FilterType filterType;
                    std::string startDate, endDate;
                    double minTemp, maxTemp, minVolatility;
                    bool uptrend;
                    
                    // Show current filtering status
                    if (filtersApplied > 0) {
                        std::cout << "=== CURRENT STATUS ===\n";
                        std::cout << "Filters applied: " << filtersApplied << "\n";
                        std::cout << "Current dataset size: " << dataForAnalysis.size() << " candlesticks\n";
                        std::cout << "Original dataset size: " << candlesticks.size() << " candlesticks\n\n";
                        
                        std::cout << "Choose filtering mode:\n";
                        std::cout << "  1. Apply to current filtered data (cumulative)\n";
                        std::cout << "  2. Apply to original data (fresh filter)\n";
                        
                        int mode;
                        while (true) {
                            std::cout << "Enter mode (1-2): ";
                            if (!(std::cin >> mode) || (mode != 1 && mode != 2)) {
                                std::cout << "Error: Please enter 1 or 2.\n";
                                UserInput::Internal::clearInputBuffer();
                                continue;
                            }
                            break;
                        }
                        
                        if (UserInput::getFilterCriteria(candlesticks, timeframe, filterType, 
                                                       startDate, endDate, minTemp, maxTemp, 
                                                       uptrend, minVolatility)) {
                            
                            UserInput::clearScreen();
                            
                            // Use move semantics for better performance
                            std::vector<Candlestick> sourceData = (mode == 1) ? dataForAnalysis : candlesticks;
                            std::vector<Candlestick> filtered = applyFilter(
                                std::move(sourceData), filterType, startDate, endDate, 
                                minTemp, maxTemp, uptrend, minVolatility
                            );
                            
                            if (!filtered.empty()) {
                                dataForAnalysis = std::move(filtered);
                                if (mode == 2) {
                                    filtersApplied = 1; // Reset counter for fresh filter
                                } else {
                                    filtersApplied++; // Increment for cumulative filter
                                }
                                
                                std::cout << "\n==== Filtered " << Utils::timeFrameToString(timeframe) 
                                          << " Chart for " << country << " ====\n";
                                std::cout << "Mode: " << (mode == 1 ? "Cumulative" : "Fresh") 
                                          << " filtering (Filter " << filtersApplied << " applied)\n\n";
                                Plotter::plotCandlesticks(dataForAnalysis, timeframe, 20);
                                
                                // Show filtered data table option
                                if (UserInput::askToContinue("view the filtered data table")) {
                                    CandlestickCalculator::printCandlestickTable(dataForAnalysis);
                                    UserInput::waitForUser();
                                }
                                
                            } else {
                                std::cout << "\n⚠ Filter resulted in no data. Keeping previous dataset.\n";
                                Plotter::plotCandlesticks(dataForAnalysis, timeframe, 20);
                            }
                        }
                    } else {
                        // First filter - standard behavior
                        if (UserInput::getFilterCriteria(candlesticks, timeframe, filterType, 
                                                       startDate, endDate, minTemp, maxTemp, 
                                                       uptrend, minVolatility)) {
                            
                            UserInput::clearScreen();
                            
                            // Use move semantics for better performance
                            std::vector<Candlestick> filtered = applyFilter(
                                std::vector<Candlestick>(candlesticks), filterType, startDate, endDate, 
                                minTemp, maxTemp, uptrend, minVolatility
                            );
                            
                            if (!filtered.empty()) {
                                dataForAnalysis = std::move(filtered);
                                filtersApplied++;
                                
                                std::cout << "\n==== Filtered " << Utils::timeFrameToString(timeframe) 
                                          << " Chart for " << country 
                                          << " (Filter " << filtersApplied << " applied) ====\n";
                                Plotter::plotCandlesticks(dataForAnalysis, timeframe, 20);
                                
                                // Show filtered data table option
                                if (UserInput::askToContinue("view the filtered data table")) {
                                    CandlestickCalculator::printCandlestickTable(dataForAnalysis);
                                    UserInput::waitForUser();
                                }
                                
                            } else {
                                std::cout << "\n⚠ Filter resulted in no data. Showing original view.\n";
                                Plotter::plotCandlesticks(dataForAnalysis, timeframe, 20);
                            }
                        }
                    }

                    if (!UserInput::askToContinue("apply another filter")) {
                        break;
                    }
                    UserInput::clearScreen();
                }
            }
            
            // --- Prediction Analysis ---
            if (UserInput::askForPredictions()) {
                UserInput::clearScreen();
                
                displayPredictions(dataForAnalysis, timeframe, country, startYear, endYear);
                
                // Option to see predictions for original data if filters were applied
                if (filtersApplied > 0 && dataForAnalysis.size() != candlesticks.size()) {
                    if (UserInput::askToContinue("see predictions for the complete (unfiltered) dataset")) {
                        UserInput::clearScreen();
                        std::cout << "=== Predictions for Complete (Unfiltered) Dataset ===\n";
                        displayPredictions(candlesticks, timeframe, country, startYear, endYear);
                    }
                }
            }
            
            // Ask if user wants to analyze another country
            if (!UserInput::askToAnalyzeAnotherCountry()) {
                break; // Exit the main loop
            }
            
        } catch (const std::exception& e) {
            std::cerr << "\nError: " << e.what() << std::endl;
            std::cout << "\nWould you like to try again with different parameters? (y/n): ";
            std::string retry;
            std::cin >> retry;
            std::transform(retry.begin(), retry.end(), retry.begin(), ::tolower);
            if (retry != "y" && retry != "yes") {
                break;
            }
            UserInput::clearScreen();
        } catch (...) {
            std::cerr << "\nAn unexpected error occurred." << std::endl;
            std::cout << "\nWould you like to try again? (y/n): ";
            std::string retry;
            std::cin >> retry;
            std::transform(retry.begin(), retry.end(), retry.begin(), ::tolower);
            if (retry != "y" && retry != "yes") {
                break;
            }
            UserInput::clearScreen();
        }
    }
    
    // Farewell message
    UserInput::clearScreen();
    std::cout << "\n" << std::string(Constants::SECTION_SEPARATOR_WIDTH_60, '=') << "\n";
    std::cout << "   THANK YOU FOR USING THE TEMPERATURE ANALYSIS TOOL!\n";
    std::cout << "Have a great day! \n\n";

    return 0;
}