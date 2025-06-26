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

    // Helper function to display prediction results
    void displayPredictions(const std::vector<Candlestick>& data, TimeFrame timeframe, 
                          const std::string& country, const std::string& dataDescription) {
        if (data.empty()) {
            std::cout << "Cannot generate predictions: No data available.\n";
            return;
        }

        std::cout << "\n=== Temperature Predictions ===\n";
        std::cout << "Based on " << dataDescription << " for " << country << "\n";
        std::cout << "Data points used: " << data.size() << " " << Utils::timeFrameToString(timeframe) << " periods\n\n";

        // Generate predictions using all three models
        double linearPred = Prediction::predictLinear(data);
        double movingAvg3 = Prediction::predictMovingAverage(data, 3);
        double movingAvg5 = Prediction::predictMovingAverage(data, 5);
        double heuristicPred = Prediction::predictHeuristic(data);

        // Calculate confidence metrics
        double rSquared = Prediction::calculateLinearRSquared(data);
        
        // Display predictions in a formatted table
        std::cout << std::left << std::setw(25) << "Prediction Model" 
                  << std::setw(15) << "Next Period" 
                  << std::setw(20) << "Best Used For" 
                  << std::setw(15) << "Confidence" << "\n";
        std::cout << std::string(75, '-') << "\n";
        
        std::cout << std::left << std::setw(25) << "Linear Regression" 
                  << std::setw(15) << (std::to_string(linearPred).substr(0,5) + "°C")
                  << std::setw(20) << "Long-term trends"
                  << std::setw(15) << ("R² = " + std::to_string(rSquared).substr(0,4)) << "\n";
        
        std::cout << std::left << std::setw(25) << "Moving Average (3-period)" 
                  << std::setw(15) << (std::to_string(movingAvg3).substr(0,5) + "°C")
                  << std::setw(20) << "Smoothed forecast"
                  << std::setw(15) << "Stable" << "\n";
        
        std::cout << std::left << std::setw(25) << "Moving Average (5-period)" 
                  << std::setw(15) << (std::to_string(movingAvg5).substr(0,5) + "°C")
                  << std::setw(20) << "Very smooth"
                  << std::setw(15) << "Very stable" << "\n";
        
        std::cout << std::left << std::setw(25) << "Momentum (Heuristic)" 
                  << std::setw(15) << (std::to_string(heuristicPred).substr(0,5) + "°C")
                  << std::setw(20) << "Recent trends"
                  << std::setw(15) << "Reactive" << "\n";

        std::cout << "\n";

        // Provide interpretation guidance
        std::cout << "=== Prediction Interpretation ===\n";
        
        // Linear regression analysis
        if (rSquared > 0.7) {
            std::cout << "✓ Linear trend is STRONG (R² > 0.7) - Linear regression prediction is reliable\n";
        } else if (rSquared > 0.4) {
            std::cout << "⚠ Linear trend is MODERATE (R² > 0.4) - Consider multiple models\n";
        } else {
            std::cout << "⚠ Linear trend is WEAK (R² < 0.4) - Linear prediction may be unreliable\n";
        }

        // Calculate recent volatility for additional context
        if (data.size() >= 3) {
            double recentVolatility = 0.0;
            int volatilityPeriods = std::min(5, static_cast<int>(data.size()));
            for (int i = data.size() - volatilityPeriods; i < static_cast<int>(data.size()); i++) {
                recentVolatility += data[i].getVolatility();
            }
            recentVolatility /= volatilityPeriods;

            std::cout << "Recent volatility: " << std::fixed << std::setprecision(1) 
                      << recentVolatility << "°C (avg of last " << volatilityPeriods << " periods)\n";
        }

        // Trend direction analysis
        if (data.size() >= 2) {
            double recentChange = data.back().getClose() - data[data.size()-2].getClose();
            std::cout << "Recent trend: " << (recentChange > 0 ? "↗ Warming" : recentChange < 0 ? "↘ Cooling" : "→ Stable");
            std::cout << " (" << std::showpos << std::fixed << std::setprecision(1) << recentChange << "°C)\n";
        }

        std::cout << "\n";

        // Model recommendation based on data characteristics
        std::cout << "=== Recommendation ===\n";
        if (rSquared > 0.6) {
            std::cout << "🎯 RECOMMENDED: Use Linear Regression (strong trend detected)\n";
        } else if (data.size() >= 5) {
            std::cout << "🎯 RECOMMENDED: Use Moving Average (data shows variability)\n";
        } else {
            std::cout << "🎯 RECOMMENDED: Use Multiple Models (limited data - compare results)\n";
        }

        // Test prediction accuracy if enough data
        if (data.size() >= 8) {
            std::cout << "\n=== Model Accuracy Test ===\n";
            std::cout << "Testing on last 3 periods of historical data:\n";
            
            std::vector<double> errors3 = Prediction::calculateMovingAverageErrors(data, 3, 3);
            std::vector<double> errors5 = Prediction::calculateMovingAverageErrors(data, 5, 3);
            
            if (!errors3.empty()) {
                double mae3 = PredictionUtils::calculateMAE(errors3);
                std::cout << "3-period Moving Average MAE: " << std::fixed << std::setprecision(2) << mae3 << "°C\n";
            }
            
            if (!errors5.empty()) {
                double mae5 = PredictionUtils::calculateMAE(errors5);
                std::cout << "5-period Moving Average MAE: " << std::fixed << std::setprecision(2) << mae5 << "°C\n";
            }
            
            std::cout << "(Lower MAE = better accuracy)\n";
        }

        std::cout << "\n";
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
            
            displayPredictions(dataForAnalysis, timeframe, country, dataDescription);
            
            // Option to see predictions for original data if filters were applied
            if (filtersApplied > 0 && dataForAnalysis.size() != candlesticks.size()) {
                if (UserInput::askToContinue("see predictions for the complete (unfiltered) dataset")) {
                    displayPredictions(candlesticks, timeframe, country, "complete dataset");
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