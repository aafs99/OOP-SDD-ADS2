#ifndef USERINPUT_H
#define USERINPUT_H

#include <string>
#include <vector>

/**
 * UserInput class to handle user input validation and collection.\
 */
class UserInput {
private:
    // List of valid country codes from the CSV
    static const std::vector<std::string> VALID_COUNTRIES;
    
    // Valid year range constants
    static const int MIN_YEAR = 1980; // Minimum year from the given CSV data
    static const int MAX_YEAR = 2019; // Maximum year from the given CSV data

public:
    /**
     * Validdate country code from user input
     * Displays available countries and validates input.
     * Returns validated country code in uppercase.
     */
    static std::string getCountryCode();
    
    /**
     * Get a valid year from user input with error handling.
     * @param prompt The prompt message to display to user
     * @param minYear Minimum allowed year 
     * @param maxYear Maximum allowed year 
     * @return Valid year within specified range
     */
    static int getYear(const std::string& prompt, int minYear = MIN_YEAR, int maxYear = MAX_YEAR);
    
    /**
     * Get a valid year range (start and end years) from user input
     * @param startYear Reference to store the validated start year
     * @param endYear Reference to store the validated end year
     */
    static void getYearRange(int& startYear, int& endYear);
    
    /**
     * Display available countries to help user choose.
     */
    static void displayAvailableCountries();
    
private:
    /**
     * Convert string to uppercase for case-insensitive comparison.
     */
    static std::string toUpperCase(const std::string& str);
    
    /**
     * Check if a country code is valid.
     */
    static bool isValidCountryCode(const std::string& country);
    
    /**
     * Clear input buffer after invalid input.
     */
    static void clearInputBuffer();
};

#endif