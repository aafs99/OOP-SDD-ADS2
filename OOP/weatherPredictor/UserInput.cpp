#include "UserInput.h"
#include "Common.h"
#include "Candlestick.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>

namespace {
    // Use a map as a single source of truth for country codes and their full names.
    const std::map<std::string, std::string> AVAILABLE_COUNTRIES = {
        {"AT", "Austria"}, {"BE", "Belgium"}, {"BG", "Bulgaria"},
        {"CH", "Switzerland"}, {"CZ", "Czech Rep."}, {"DE", "Germany"},
        {"DK", "Denmark"}, {"EE", "Estonia"}, {"ES", "Spain"},
        {"FI", "Finland"}, {"FR", "France"}, {"GB", "Great Britain"},
        {"GR", "Greece"}, {"HR", "Croatia"}, {"HU", "Hungary"},
        {"IE", "Ireland"}, {"IT", "Italy"}, {"LT", "Lithuania"},
        {"LU", "Luxembourg"}, {"LV", "Latvia"}, {"NL", "Netherlands"},
        {"NO", "Norway"}, {"PL", "Poland"}, {"PT", "Portugal"},
        {"RO", "Romania"}, {"SE", "Sweden"}, {"SI", "Slovenia"},
        {"SK", "Slovakia"}
    };
}

namespace UserInput {

// Forward declaration from Internal namespace
namespace Internal {
    bool isValidCountryCode(const std::string& country);
}

std::string getCountryCode() {
    std::string country;
    bool validInput = false;
    
    std::cout << "\n=== Country Selection ===\n";
    displayAvailableCountries();
    
    while (!validInput) {
        std::cout << "\nEnter country code (2 letters): ";
        std::cin >> country;
        
        country = Internal::toUpperCase(country);
        
        if (country.length() != 2) {
            std::cout << "Error: Country code must be exactly 2 letters.\n";
            Internal::clearInputBuffer();
            continue;
        }
        
        if (!Internal::isValidCountryCode(country)) {
            std::cout << "Error: '" << country << "' is not a valid country code.\n";
            std::cout << "Please choose from the available countries listed above.\n";
            Internal::clearInputBuffer();
            continue;
        }
        
        auto it = AVAILABLE_COUNTRIES.find(country);
        if (it != AVAILABLE_COUNTRIES.end()) {
            std::cout << "\n✓ Selected: " << country << " (" << it->second << ")\n";
        } else {
            std::cout << "\n✓ Selected: " << country << "\n";
        }
        validInput = true;
    }
    
    return country;
}

void getYearRange(int& startYear, int& endYear) {
    std::cout << "\n=== Year Range Selection ===\n";
    std::cout << "Available data range: " << Constants::MIN_YEAR << "-" << Constants::MAX_YEAR << "\n\n";
    
    startYear = Internal::getYear("Enter start year", Constants::MIN_YEAR, Constants::MAX_YEAR);
    
    bool validRange = false;
    while (!validRange) {
        endYear = Internal::getYear("Enter end year", startYear, Constants::MAX_YEAR);
        
        if (endYear < startYear) {
            std::cout << "Error: End year (" << endYear << ") cannot be before start year (" << startYear << ").\n";
            continue;
        }
        validRange = true;
    }
    
    std::cout << "\n✓ Selected range: " << startYear << "-" << endYear 
              << " (" << (endYear - startYear + 1) << " years)\n";
}

TimeFrame getTimeFrame() {
    int choice;
    
    std::cout << "\n=== Timeframe Selection ===\n";
    Internal::displayAvailableTimeFrames();
    
    while (true) {
        std::cout << "\nEnter your choice (1-3): ";
        
        if (!(std::cin >> choice)) {
            std::cout << "Error: Please enter a valid number.\n";
            Internal::clearInputBuffer();
            continue;
        }
        
        switch (choice) {
            case 1: 
                std::cout << "\n✓ Selected: Yearly aggregation\n";
                return TimeFrame::Yearly;
            case 2: 
                std::cout << "\n✓ Selected: Monthly aggregation\n";
                return TimeFrame::Monthly;
            case 3: 
                std::cout << "\n✓ Selected: Daily aggregation\n";
                return TimeFrame::Daily;
            default:
                std::cout << "Error: Please enter 1, 2, or 3.\n";
                continue;
        }
    }
}

bool askForFiltering() {
    std::string choice;
    
    std::cout << "\n=== Data Filtering ===\n";
    std::cout << "Filtering allows you to focus your analysis on specific aspects of the data:\n";
    std::cout << "  • Date Range: Analyze specific time periods\n";
    std::cout << "  • Temperature Range: Focus on periods with certain temperature levels\n";
    std::cout << "  • Trend Direction: Analyze only warming or cooling periods\n";
    std::cout << "  • Volatility: Focus on periods with high temperature variation\n";
    std::cout << "\nWould you like to apply a filter to the data? (y/n): ";
    
    while (true) {
        std::cin >> choice;
        std::transform(choice.begin(), choice.end(), choice.begin(), ::tolower);
        
        if (choice == "y" || choice == "yes") return true;
        if (choice == "n" || choice == "no") return false;
        
        std::cout << "Please enter 'y' for yes or 'n' for no: ";
        Internal::clearInputBuffer();
    }
}

bool askForPredictions() {
    std::string choice;
    
    std::cout << "\n=== ENHANCED TASK 4: Temperature Predictions ===\n";
    std::cout << "Generate advanced temperature forecasts with confidence metrics:\n";
    std::cout << "  • Linear Regression: R² confidence metric (trend strength)\n";
    std::cout << "  • Moving Average: Stability confidence (based on volatility)\n";
    std::cout << "  • Momentum Model: Consistency confidence (trend reliability)\n";
    std::cout << "  • Cross-Validation: Model accuracy assessment (if sufficient data)\n";
    std::cout << "  • Prediction Charts: Visual comparison of actual vs predicted (Figure 4 style)\n";
    std::cout << "\nWould you like to generate enhanced temperature predictions? (y/n): ";
    
    while (true) {
        std::cin >> choice;
        std::transform(choice.begin(), choice.end(), choice.begin(), ::tolower);
        
        if (choice == "y" || choice == "yes") return true;
        if (choice == "n" || choice == "no") return false;
        
        std::cout << "Please enter 'y' for yes or 'n' for no: ";
        Internal::clearInputBuffer();
    }
}

bool askToContinue(const std::string& operation) {
    std::string choice;
    
    std::cout << "\nWould you like to " << operation << "? (y/n): ";
    
    while (true) {
        std::cin >> choice;
        std::transform(choice.begin(), choice.end(), choice.begin(), ::tolower);
        
        if (choice == "y" || choice == "yes") return true;
        if (choice == "n" || choice == "no") return false;
        
        std::cout << "Please enter 'y' for yes or 'n' for no: ";
        Internal::clearInputBuffer();
    }
}

bool getFilterCriteria(const std::vector<Candlestick>& candlesticks, TimeFrame timeframe,
                      FilterType& filterType, std::string& startDate, std::string& endDate,
                      double& minTemp, double& maxTemp, bool& uptrend, double& minVolatility) {
    if (candlesticks.empty()) {
        std::cout << "Error: No data available for filtering.\n";
        return false;
    }
    
    std::cout << "\n=== Filter Selection ===\n";
    std::cout << "Available filter types:\n";
    std::cout << "  1. Date Range - Filter by specific date range\n";
    std::cout << "  2. Temperature Range - Filter by temperature values\n";
    std::cout << "  3. Trend Direction - Filter by upward or downward trends\n";
    std::cout << "  4. Volatility - Filter by temperature volatility\n";
    
    int filterChoice;
    while (true) {
        std::cout << "\nEnter filter type (1-4): ";
        if (!(std::cin >> filterChoice) || filterChoice < 1 || filterChoice > 4) {
            std::cout << "Error: Please enter a number between 1 and 4.\n";
            Internal::clearInputBuffer();
            continue;
        }
        break;
    }
    
    filterType = static_cast<FilterType>(filterChoice);
    
    switch (filterType) {
        case FilterType::DateRange:
            return Internal::getDateRangeFilter(candlesticks, timeframe, startDate, endDate);
        case FilterType::TemperatureRange:
            return Internal::getTemperatureRangeFilter(candlesticks, minTemp, maxTemp);
        case FilterType::Trend:
            return Internal::getTrendFilter(uptrend);
        case FilterType::Volatility:
            return Internal::getVolatilityFilter(candlesticks, minVolatility);
        default:
            return false;
    }
}

void displayAvailableCountries() {
    std::cout << "Available countries:\n";
    int i = 0;
    for (const auto& pair : AVAILABLE_COUNTRIES) {
        // Print "  CODE (Full Name)      "
        std::cout << "  " << std::left << std::setw(4) << pair.first
                  << std::setw(18) << ("(" + pair.second + ")");
        
        if (++i % 3 == 0) { // Print 3 countries per line
            std::cout << "\n";
        }
    }
    if (i % 3 != 0) {
        std::cout << "\n";
    }
}

void clearScreen() {
    // Use ANSI escape codes to clear screen - works on most modern terminals
    std::cout << "\033[2J\033[H" << std::flush;
    
    // Alternative: Use system-specific commands
    // #ifdef _WIN32
    //     system("cls");
    // #else
    //     system("clear");
    // #endif
}

void waitForUser() {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

bool askForPredictionChart() {
    std::string choice;
    
    std::cout << "\n=== PREDICTION COMPARISON CHART ===\n";
    std::cout << "Generate a visual chart comparing actual temperatures with predictions\n";
    std::cout << "from all three models across the historical period.\n";
    std::cout << "\nThis creates 'Figure 4' style visualization showing:\n";
    std::cout << "• Actual temperatures (black dots)\n";
    std::cout << "• Linear model predictions (blue triangles)\n";
    std::cout << "• Moving average predictions (green squares)\n";
    std::cout << "• Heuristic model predictions (red diamonds)\n";
    std::cout << "\nWould you like to generate the prediction comparison chart? (y/n): ";
    
    while (true) {
        std::cin >> choice;
        std::transform(choice.begin(), choice.end(), choice.begin(), ::tolower);
        
        if (choice == "y" || choice == "yes") return true;
        if (choice == "n" || choice == "no") return false;
        
        std::cout << "Please enter 'y' for yes or 'n' for no: ";
        Internal::clearInputBuffer();
    }
}

bool askToAnalyzeAnotherCountry() {
    std::string choice;
    
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Analysis Complete!\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << "\nWould you like to analyze another country? (y/n): ";
    
    while (true) {
        std::cin >> choice;
        std::transform(choice.begin(), choice.end(), choice.begin(), ::tolower);
        
        if (choice == "y" || choice == "yes") {
            clearScreen();
            return true;
        }
        if (choice == "n" || choice == "no") {
            return false;
        }
        
        std::cout << "Please enter 'y' for yes or 'n' for no: ";
        Internal::clearInputBuffer();
    }
}


namespace Internal {

void displayAvailableTimeFrames() {
    std::cout << "Available timeframes:\n";
    std::cout << "  1. Yearly   - One candlestick per year (recommended for long-term trends)\n";
    std::cout << "  2. Monthly  - One candlestick per month (good for seasonal patterns)\n";
    std::cout << "  3. Daily    - One candlestick per day (detailed short-term analysis)\n";
    std::cout << "\nNote: Daily timeframe will generate many candlesticks and may be\n";
    std::cout << "compressed for display. Consider using shorter date ranges with daily data.\n";
}

int getYear(const std::string& prompt, int minYear, int maxYear) {
    int year;
    
    while (true) {
        std::cout << prompt << " (" << minYear << "-" << maxYear << "): ";
        
        if (!(std::cin >> year)) {
            std::cout << "Error: Please enter a valid number.\n";
            clearInputBuffer();
            continue;
        }
        
        if (year < minYear || year > maxYear) {
            std::cout << "Error: Year must be between " << minYear << " and " << maxYear << ".\n";
            continue;
        }
        
        return year;
    }
}

bool getDateRangeFilter(const std::vector<Candlestick>& candlesticks, TimeFrame timeframe,
                       std::string& startDate, std::string& endDate) {
    std::cout << "\n=== Date Range Filter ===\n";
    std::cout << "Available date range: " << candlesticks.front().getDate() 
              << " to " << candlesticks.back().getDate() << "\n";
    
    std::cout << "Enter start date (YYYY-MM-DD): ";
    std::cin >> startDate;
    
    std::cout << "Enter end date (YYYY-MM-DD): ";
    std::cin >> endDate;
    
    if (startDate.length() != 10 || endDate.length() != 10 ||
        startDate[4] != '-' || startDate[7] != '-' ||
        endDate[4] != '-' || endDate[7] != '-') {
        std::cout << "Error: Invalid date format. Use YYYY-MM-DD.\n";
        return false;
    }
    
    if (startDate > endDate) {
        std::cout << "Error: Start date cannot be after end date.\n";
        return false;
    }
    
    return true;
}

bool getTemperatureRangeFilter(const std::vector<Candlestick>& candlesticks,
                              double& minTemp, double& maxTemp) {
    double dataMinTemp = candlesticks[0].getLow();
    double dataMaxTemp = candlesticks[0].getHigh();
    
    for (const auto& candle : candlesticks) {
        dataMinTemp = std::min({dataMinTemp, candle.getLow(), candle.getClose()});
        dataMaxTemp = std::max({dataMaxTemp, candle.getHigh(), candle.getClose()});
    }
    
    std::cout << "\n=== Temperature Range Filter ===\n";
    std::cout << "Available temperature range in data: " << std::fixed << std::setprecision(1) 
              << dataMinTemp << "°C to " << dataMaxTemp << "°C\n";
    std::cout << "This filter keeps candlesticks where the average temperature (close value) is within your range.\n";
    
    while (true) {
        std::cout << "Enter minimum temperature (°C): ";
        if (!(std::cin >> minTemp)) {
            std::cout << "Error: Please enter a valid number.\n";
            clearInputBuffer();
            continue;
        }
        
        std::cout << "Enter maximum temperature (°C): ";
        if (!(std::cin >> maxTemp)) {
            std::cout << "Error: Please enter a valid number.\n";
            clearInputBuffer();
            continue;
        }
        
        if (minTemp > maxTemp) {
            std::cout << "Error: Minimum temperature cannot be greater than maximum temperature.\n";
            continue;
        }
        
        break;
    }
    
    return true;
}

bool getTrendFilter(bool& uptrend) {
    int choice;
    
    std::cout << "\n=== Trend Direction Filter ===\n";
    std::cout << "Filter options:\n";
    std::cout << "  1. Upward trends (close >= open)\n";
    std::cout << "  2. Downward trends (close < open)\n";
    
    while (true) {
        std::cout << "Enter choice (1-2): ";
        if (!(std::cin >> choice) || (choice != 1 && choice != 2)) {
            std::cout << "Error: Please enter 1 or 2.\n";
            clearInputBuffer();
            continue;
        }
        
        uptrend = (choice == 1);
        break;
    }
    
    return true;
}

bool getVolatilityFilter(const std::vector<Candlestick>& candlesticks, double& minVolatility) {
    std::vector<double> volatilities;
    volatilities.reserve(candlesticks.size());
    for (const auto& candle : candlesticks) {
        volatilities.push_back(candle.getVolatility());
    }
    
    std::sort(volatilities.begin(), volatilities.end());
    
    std::cout << "\n=== Volatility Filter ===\n";
    std::cout << "Volatility is the difference between high and low temperatures.\n";
    
    while (true) {
        std::cout << "\nEnter minimum volatility (°C) to keep: ";
        if (!(std::cin >> minVolatility) || minVolatility < 0) {
            std::cout << "Error: Please enter a positive number.\n";
            clearInputBuffer();
            continue;
        }
        break;
    }
    
    return true;
}


std::string toUpperCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool isValidCountryCode(const std::string& country) {
    // Check if the country code exists as a key in the map.
    return AVAILABLE_COUNTRIES.count(country) > 0;
}

void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

} 
}