#include "UserInput.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <cctype>

// Define valid country codes based on CSV structure
const std::vector<std::string> UserInput::VALID_COUNTRIES = {
    "AT", "BE", "BG", "CH", "CZ", "DE", "DK", "EE", "ES", "FI", 
    "FR", "GB", "GR", "HR", "HU", "IE", "IT", "LT", "LU", "LV", 
    "NL", "NO", "PL", "PT", "RO", "SE", "SI", "SK"
};

std::string UserInput::getCountryCode() {
    std::string country;
    bool validInput = false;
    
    std::cout << "\n=== Country Selection ===\n";
    displayAvailableCountries();
    
    while (!validInput) {
        std::cout << "\nEnter country code (2 letters): ";
        std::cin >> country;
        
        // Convert to uppercase for consistency
        country = toUpperCase(country);
        
        // Validate country code
        if (country.length() != 2) {
            std::cout << "Error: Country code must be exactly 2 letters.\n";
            clearInputBuffer();
            continue;
        }
        
        if (!isValidCountryCode(country)) {
            std::cout << "Error: '" << country << "' is not a valid country code.\n";
            std::cout << "Please choose from the available countries listed above.\n";
            clearInputBuffer();
            continue;
        }
        validInput = true;
    }
    
    return country;
}

int UserInput::getYear(const std::string& prompt, int minYear, int maxYear) {
    int year;
    bool validInput = false;
    
    while (!validInput) {
        std::cout << prompt << " (" << minYear << "-" << maxYear << "): ";
        
        if (!(std::cin >> year)) {
            // Handle non-numeric input
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

void UserInput::getYearRange(int& startYear, int& endYear) {
    std::cout << "\n=== Year Range Selection ===\n";
    
    // Get start year
    startYear = getYear("Enter start year", MIN_YEAR, MAX_YEAR);
    
    // Get end year (must be >= start year)
    bool validRange = false;
    while (!validRange) {
        endYear = getYear("Enter end year", startYear, MAX_YEAR);
        
        if (endYear < startYear) {
            std::cout << "Error: End year (" << endYear << ") cannot be before start year (" << startYear << ").\n";
            continue;
        }
        validRange = true;
    }
}

void UserInput::displayAvailableCountries() {
    std::cout << "Available countries:\n";
    
    // Display countries in rows of 4 for better readability
    for (size_t i = 0; i < VALID_COUNTRIES.size(); ++i) {
        std::cout << "  " << VALID_COUNTRIES[i];
        
        // Add country names for clarity
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
        
        // Line break every 2 countries for readability
        if ((i + 1) % 2 == 0) {
            std::cout << "\n";
        }
    }
    
    // Add final newline if needed
    if (VALID_COUNTRIES.size() % 2 != 0) {
        std::cout << "\n";
    }
}

std::string UserInput::toUpperCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool UserInput::isValidCountryCode(const std::string& country) {
    return std::find(VALID_COUNTRIES.begin(), VALID_COUNTRIES.end(), country) != VALID_COUNTRIES.end();
}

void UserInput::clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}