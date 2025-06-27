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
    /**
     * @brief Applies the specified filter to candlestick data
     * FIXED: Proper move semantics and simplified logic
     */
    std::vector<Candlestick> applyFilter(
        std::vector<Candlestick> data, // Accept by value for proper move semantics
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
                return data; // FIXED: Remove unnecessary std::move
        }
    }

    /**
     * @brief Displays chart and optional data table
     * FIXED: Eliminates code duplication
     */
    void displayChartAndTable(const std::vector<Candlestick>& data, TimeFrame timeframe, 
                             const std::string& country, const std::string& chartTitle = "") {
        if (!chartTitle.empty()) {
            std::cout << "\n==== " << chartTitle << " ====\n\n";
        }
        Plotter::plotCandlesticks(data, timeframe, 20);
        
        if (UserInput::askToContinue("view the detailed candlestick data table")) {
            CandlestickCalculator::printCandlestickTable(data);
            UserInput::waitForUser();
        }
    }

    /**
     * @brief Helper function to display prediction analysis
     * IMPROVED: Better error handling and user feedback
     */
    void displayPredictions(const std::vector<Candlestick>& data, TimeFrame timeframe, 
                          const std::string& country, int startYear, int endYear) {
        if (data.empty()) {
            std::cout << "Cannot generate predictions: No data available.\n";
            return;
        }

        if (data.size() < 2) {
            std::cout << "Cannot generate predictions: Need at least 2 data points for analysis.\n";
            std::cout << "Current dataset has " << data.size() << " point(s).\n";
            return;
        }

        // Use the enhanced prediction display with configuration
        PredictionConfig config;
        config.showValidation = (data.size() >= 4); // Only show validation if enough data
        config.showChart = (data.size() >= 4);      // Only show chart if enough data
        
        Prediction::displayEnhancedPredictionResults(data, timeframe, country, startYear, endYear, config);
    }

    /**
     * @brief Handles the filtering workflow
     * IMPROVED: Simplified logic and better user experience
     */
    bool handleFiltering(std::vector<Candlestick>& dataForAnalysis, 
                        const std::vector<Candlestick>& originalData,
                        TimeFrame timeframe, const std::string& country) {
        if (!UserInput::askForFiltering()) {
            return false;
        }

        UserInput::clearScreen();
        int filtersApplied = 0;
        
        while (true) {
            FilterType filterType;
            std::string startDate, endDate;
            double minTemp, maxTemp, minVolatility;
            bool uptrend;
            
            // Show current status if filters have been applied
            if (filtersApplied > 0) {
                std::cout << "=== CURRENT FILTERING STATUS ===\n";
                std::cout << "Filters applied: " << filtersApplied << "\n";
                std::cout << "Current dataset size: " << dataForAnalysis.size() << " candlesticks\n";
                std::cout << "Original dataset size: " << originalData.size() << " candlesticks\n\n";
                
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
                
                if (!UserInput::getFilterCriteria(originalData, timeframe, filterType, 
                                                startDate, endDate, minTemp, maxTemp, 
                                                uptrend, minVolatility)) {
                    continue;
                }
                
                UserInput::clearScreen();
                
                // Apply filter to appropriate source data
                std::vector<Candlestick> sourceData = (mode == 1) ? dataForAnalysis : originalData;
                std::vector<Candlestick> filtered = applyFilter(
                    std::move(sourceData), filterType, startDate, endDate, 
                    minTemp, maxTemp, uptrend, minVolatility
                );
                
                if (!filtered.empty()) {
                    dataForAnalysis = std::move(filtered);
                    filtersApplied = (mode == 2) ? 1 : filtersApplied + 1;
                    
                    std::string chartTitle = "Filtered " + Utils::timeFrameToString(timeframe) + 
                                           " Chart for " + country + " (Filter " + 
                                           std::to_string(filtersApplied) + " applied)";
                    displayChartAndTable(dataForAnalysis, timeframe, country, chartTitle);
                } else {
                    std::cout << "\n⚠ Filter resulted in no data. Keeping previous dataset.\n";
                    displayChartAndTable(dataForAnalysis, timeframe, country);
                }
            } else {
                // First filter
                if (!UserInput::getFilterCriteria(originalData, timeframe, filterType, 
                                                startDate, endDate, minTemp, maxTemp, 
                                                uptrend, minVolatility)) {
                    continue;
                }
                
                UserInput::clearScreen();
                
                std::vector<Candlestick> filtered = applyFilter(
                    originalData, filterType, startDate, endDate, 
                    minTemp, maxTemp, uptrend, minVolatility
                );
                
                if (!filtered.empty()) {
                    dataForAnalysis = std::move(filtered);
                    filtersApplied = 1;
                    
                    std::string chartTitle = "Filtered " + Utils::timeFrameToString(timeframe) + 
                                           " Chart for " + country + " (Filter 1 applied)";
                    displayChartAndTable(dataForAnalysis, timeframe, country, chartTitle);
                } else {
                    std::cout << "\n⚠ Filter resulted in no data. Showing original view.\n";
                    displayChartAndTable(dataForAnalysis, timeframe, country);
                }
            }

            if (!UserInput::askToContinue("apply another filter")) {
                break;
            }
            UserInput::clearScreen();
        }
        
        return filtersApplied > 0;
    }

    /**
     * @brief Handles the prediction workflow
     * IMPROVED: Better separation of concerns
     */
    void handlePredictions(const std::vector<Candlestick>& dataForAnalysis,
                          const std::vector<Candlestick>& originalData,
                          TimeFrame timeframe, const std::string& country,
                          int startYear, int endYear, bool wasFiltered) {
        if (!UserInput::askForPredictions()) {
            return;
        }

        UserInput::clearScreen();
        
        displayPredictions(dataForAnalysis, timeframe, country, startYear, endYear);
        
        // Option to see predictions for original data if filters were applied
        if (wasFiltered && dataForAnalysis.size() != originalData.size()) {
            if (UserInput::askToContinue("see predictions for the complete (unfiltered) dataset")) {
                UserInput::clearScreen();
                std::cout << "=== Predictions for Complete (Unfiltered) Dataset ===\n";
                displayPredictions(originalData, timeframe, country, startYear, endYear);
            }
        }
    }

    /**
     * @brief Validates and loads data with comprehensive error checking
     * IMPROVED: Better error messages and data validation
     */
    bool loadAndValidateData(const std::string& filename, const std::string& country,
                           int startYear, int endYear, std::vector<TemperatureRecord>& records) {
        std::cout << "=== LOADING DATA ===\n";
        std::cout << "Loading temperature data for " << country 
                  << " from " << startYear << " to " << endYear << "...\n";
        
        records = DataLoader::loadCSV(filename, country, startYear, endYear);
        
        if (records.empty()) {
            std::cout << "\n❌ Error: No data found for the specified criteria.\n";
            std::cout << "Possible issues:\n";
            std::cout << "  • Country code '" << country << "' might not exist in the dataset\n";
            std::cout << "  • Year range " << startYear << "-" << endYear << " might have no data\n";
            std::cout << "  • Data file might be corrupted or in wrong format\n";
            std::cout << "\nPlease try different parameters.\n";
            return false;
        }

        std::cout << "✅ Successfully loaded " << records.size() << " temperature records\n";
        
        // Additional data quality checks
        if (records.size() < 10) {
            std::cout << "⚠ Warning: Very limited data (" << records.size() 
                      << " records). Results may be less reliable.\n";
        }
        
        return true;
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
            
            // Load and validate data
            std::vector<TemperatureRecord> records;
            if (!loadAndValidateData(filename, country, startYear, endYear, records)) {
                UserInput::waitForUser();
                continue;
            }

            // --- Candlestick Calculation ---
            std::cout << "\n=== PROCESSING DATA ===\n";
            std::cout << "Computing candlestick aggregation using " 
                      << Utils::timeFrameToString(timeframe) << " timeframe...\n";
            
            const std::vector<Candlestick> candlesticks = CandlestickCalculator::computeCandlesticks(records, timeframe);
            
            if (candlesticks.empty()) {
                std::cout << "❌ Error: Failed to compute candlestick data from records.\n";
                std::cout << "This might indicate a data processing issue.\n";
                UserInput::waitForUser();
                continue;
            }
            
            std::cout << "✅ Successfully computed " << candlesticks.size() << " candlestick entries.\n";
            UserInput::waitForUser();
            UserInput::clearScreen();

            // --- Initial Plot of Full Dataset ---
            std::string initialChartTitle = Utils::timeFrameToString(timeframe) + 
                                          " Candlestick Chart for " + country + 
                                          " (" + std::to_string(startYear) + "-" + std::to_string(endYear) + ")";
            displayChartAndTable(candlesticks, timeframe, country, initialChartTitle);

            // --- Enhanced Filtering Workflow ---
            std::vector<Candlestick> dataForAnalysis = candlesticks;
            bool wasFiltered = handleFiltering(dataForAnalysis, candlesticks, timeframe, country);
            
            // --- Enhanced Prediction Analysis ---
            handlePredictions(dataForAnalysis, candlesticks, timeframe, country, 
                            startYear, endYear, wasFiltered);
            
            // Ask if user wants to analyze another country
            if (!UserInput::askToAnalyzeAnotherCountry()) {
                break;
            }
            
        } catch (const std::bad_alloc& e) {
            std::cerr << "\n❌ Memory Error: " << e.what() << std::endl;
            std::cout << "The dataset might be too large for available memory.\n";
            std::cout << "Try using a smaller date range or different timeframe.\n";
            
            if (!UserInput::askToContinue("try again with different parameters")) {
                break;
            }
            UserInput::clearScreen();
            
        } catch (const std::ios_base::failure& e) {
            std::cerr << "\n❌ File I/O Error: " << e.what() << std::endl;
            std::cout << "There was a problem reading the data file.\n";
            
            if (!UserInput::askToContinue("try again")) {
                break;
            }
            UserInput::clearScreen();
            
        } catch (const std::exception& e) {
            std::cerr << "\n❌ Error: " << e.what() << std::endl;
            std::cout << "\nThis might be due to:\n";
            std::cout << "  • Invalid input parameters\n";
            std::cout << "  • Data processing issues\n";
            std::cout << "  • Memory constraints\n";
            
            if (!UserInput::askToContinue("try again with different parameters")) {
                break;
            }
            UserInput::clearScreen();
            
        } catch (...) {
            std::cerr << "\n❌ An unexpected error occurred." << std::endl;
            std::cout << "This indicates a serious system issue.\n";
            
            if (!UserInput::askToContinue("try again")) {
                break;
            }
            UserInput::clearScreen();
        }
    }
    
    // Farewell message
    UserInput::clearScreen();
    std::cout << "\n" << std::string(Constants::SECTION_SEPARATOR_WIDTH_60, '=') << "\n";
    std::cout << "   THANK YOU FOR USING THE TEMPERATURE ANALYSIS TOOL!\n";
    std::cout << "                    Have a great day! 🌡️\n";
    std::cout << std::string(Constants::SECTION_SEPARATOR_WIDTH_60, '=') << "\n\n";

    return 0;
}