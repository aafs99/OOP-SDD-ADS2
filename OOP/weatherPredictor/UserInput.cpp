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

namespace {
    const std::vector<std::string> VALID_COUNTRIES = {
        "AT", "BE", "BG", "CH", "CZ", "DE", "DK", "EE", "ES", "FI", 
        "FR", "GB", "GR", "HR", "HU", "IE", "IT", "LT", "LU", "LV", 
        "NL", "NO", "PL", "PT", "RO", "SE", "SI", "SK"
    };
}

namespace UserInput {

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
        validInput = true;
    }
    
    return country;
}

void getYearRange(int& startYear, int& endYear) {
    std::cout << "\n=== Year Range Selection ===\n";
    
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
            case 1: return TimeFrame::Yearly;
            case 2: return TimeFrame::Monthly;
            case 3: return TimeFrame::Daily;
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
        
        if (choice == "y" || choice == "yes") {
            return true;
        } else if (choice == "n" || choice == "no") {
            return false;
        } else {
            std::cout << "Please enter 'y' for yes or 'n' for no: ";
            Internal::clearInputBuffer();
        }
    }
}

bool askToContinue(const std::string& operation) {
    std::string choice;
    
    std::cout << "\nWould you like to " << operation << "? (y/n): ";
    
    while (true) {
        std::cin >> choice;
        std::transform(choice.begin(), choice.end(), choice.begin(), ::tolower);
        
        if (choice == "y" || choice == "yes") {
            return true;
        } else if (choice == "n" || choice == "no") {
            return false;
        } else {
            std::cout << "Please enter 'y' for yes or 'n' for no: ";
            Internal::clearInputBuffer();
        }
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
    
    for (size_t i = 0; i < VALID_COUNTRIES.size(); ++i) {
        std::cout << "  " << VALID_COUNTRIES[i];
        
        if (VALID_COUNTRIES[i] == "AT") std::cout << " (Austria)      ";
        else if (VALID_COUNTRIES[i] == "BE") std::cout << " (Belgium)      ";
        else if (VALID_COUNTRIES[i] == "BG") std::cout << " (Bulgaria)     ";
        else if (VALID_COUNTRIES[i] == "CH") std::cout << " (Switzerland)  ";
        else if (VALID_COUNTRIES[i] == "CZ") std::cout << " (Czech Rep.)   ";
        else if (VALID_COUNTRIES[i] == "DE") std::cout << " (Germany)      ";
        else if (VALID_COUNTRIES[i] == "DK") std::cout << " (Denmark)      ";
        else if (VALID_COUNTRIES[i] == "EE") std::cout << " (Estonia)      ";
        else if (VALID_COUNTRIES[i] == "ES") std::cout << " (Spain)        ";
        else if (VALID_COUNTRIES[i] == "FI") std::cout << " (Finland)      ";
        else if (VALID_COUNTRIES[i] == "FR") std::cout << " (France)       ";
        else if (VALID_COUNTRIES[i] == "GB") std::cout << " (Great Britain)";
        else if (VALID_COUNTRIES[i] == "GR") std::cout << " (Greece)       ";
        else if (VALID_COUNTRIES[i] == "HR") std::cout << " (Croatia)      ";
        else if (VALID_COUNTRIES[i] == "HU") std::cout << " (Hungary)      ";
        else if (VALID_COUNTRIES[i] == "IE") std::cout << " (Ireland)      ";
        else if (VALID_COUNTRIES[i] == "IT") std::cout << " (Italy)        ";
        else if (VALID_COUNTRIES[i] == "LT") std::cout << " (Lithuania)    ";
        else if (VALID_COUNTRIES[i] == "LU") std::cout << " (Luxembourg)   ";
        else if (VALID_COUNTRIES[i] == "LV") std::cout << " (Latvia)       ";
        else if (VALID_COUNTRIES[i] == "NL") std::cout << " (Netherlands)  ";
        else if (VALID_COUNTRIES[i] == "NO") std::cout << " (Norway)       ";
        else if (VALID_COUNTRIES[i] == "PL") std::cout << " (Poland)       ";
        else if (VALID_COUNTRIES[i] == "PT") std::cout << " (Portugal)     ";
        else if (VALID_COUNTRIES[i] == "RO") std::cout << " (Romania)      ";
        else if (VALID_COUNTRIES[i] == "SE") std::cout << " (Sweden)       ";
        else if (VALID_COUNTRIES[i] == "SI") std::cout << " (Slovenia)     ";
        else if (VALID_COUNTRIES[i] == "SK") std::cout << " (Slovakia)     ";
        
        if ((i + 1) % 2 == 0) {
            std::cout << "\n";
        }
    }
    
    if (VALID_COUNTRIES.size() % 2 != 0) {
        std::cout << "\n";
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
    bool validInput = false;
    
    while (!validInput) {
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
        
        validInput = true;
    }
    
    return year;
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
        dataMinTemp = std::min({dataMinTemp, candle.getOpen(), candle.getHigh(), 
                               candle.getLow(), candle.getClose()});
        dataMaxTemp = std::max({dataMaxTemp, candle.getOpen(), candle.getHigh(), 
                               candle.getLow(), candle.getClose()});
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
        
        if (maxTemp < dataMinTemp || minTemp > dataMaxTemp) {
            std::cout << "Warning: Your range [" << minTemp << "°C, " << maxTemp 
                      << "°C] is outside the data range [" << dataMinTemp << "°C, " << dataMaxTemp << "°C].\n";
            std::cout << "This will result in no data. Continue anyway? (y/n): ";
            std::string confirm;
            std::cin >> confirm;
            std::transform(confirm.begin(), confirm.end(), confirm.begin(), ::tolower);
            if (confirm != "y" && confirm != "yes") {
                continue;
            }
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
    for (const auto& candle : candlesticks) {
        volatilities.push_back(candle.getVolatility());
    }
    
    std::sort(volatilities.begin(), volatilities.end());
    double minVol = volatilities.front();
    double maxVol = volatilities.back();
    double avgVol = 0.0;
    for (double v : volatilities) avgVol += v;
    avgVol /= volatilities.size();
    
    double p25 = volatilities[volatilities.size() * 0.25];
    double p50 = volatilities[volatilities.size() * 0.50];
    double p75 = volatilities[volatilities.size() * 0.75];
    
    std::cout << "\n=== Volatility Filter ===\n";
    std::cout << "Volatility is the difference between high and low temperatures.\n";
    std::cout << "Volatility statistics in your data:\n";
    std::cout << "  Minimum: " << std::fixed << std::setprecision(1) << minVol << "°C\n";
    std::cout << "  25th percentile: " << p25 << "°C\n";
    std::cout << "  Median (50th): " << p50 << "°C\n";
    std::cout << "  75th percentile: " << p75 << "°C\n";
    std::cout << "  Maximum: " << maxVol << "°C\n";
    std::cout << "  Average: " << avgVol << "°C\n";
    std::cout << "\nSuggestions:\n";
    std::cout << "  - Use " << p50 << "°C to keep the most volatile 50% of periods\n";
    std::cout << "  - Use " << p75 << "°C to keep only the most volatile 25% of periods\n";
    
    while (true) {
        std::cout << "\nEnter minimum volatility (°C): ";
        if (!(std::cin >> minVolatility) || minVolatility < 0) {
            std::cout << "Error: Please enter a positive number.\n";
            clearInputBuffer();
            continue;
        }
        
        int remaining = 0;
        for (double v : volatilities) {
            if (v >= minVolatility) remaining++;
        }
        
        double percentage = (100.0 * remaining) / volatilities.size();
        std::cout << "This will keep " << remaining << " out of " << volatilities.size() 
                  << " periods (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
        
        if (remaining == 0) {
            std::cout << "Warning: This will filter out all data points.\n";
            std::cout << "Continue anyway? (y/n): ";
            std::string confirm;
            std::cin >> confirm;
            std::transform(confirm.begin(), confirm.end(), confirm.begin(), ::tolower);
            if (confirm != "y" && confirm != "yes") {
                continue;
            }
        }
        
        if (remaining == static_cast<int>(volatilities.size())) {
            std::cout << "Note: This will not filter any data points.\n";
            std::cout << "Continue anyway? (y/n): ";
            std::string confirm;
            std::cin >> confirm;
            std::transform(confirm.begin(), confirm.end(), confirm.begin(), ::tolower);
            if (confirm != "y" && confirm != "yes") {
                continue;
            }
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
    return std::find(VALID_COUNTRIES.begin(), VALID_COUNTRIES.end(), country) != VALID_COUNTRIES.end();
}

void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

} // namespace Internal
} // namespace UserInput