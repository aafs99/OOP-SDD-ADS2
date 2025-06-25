#include "DataLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<TemperatureRecord> DataLoader::loadCSV(const std::string& filename, const std::string& countryCode, int startYear, int endYear) {
    std::vector<TemperatureRecord> data;
    std::ifstream file(filename);
    
    // Check if file can be opened
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
        std::cerr << "Please ensure the file exists in the same directory as the executable." << std::endl;
        return data;
    }
    
    // Read and parse header to find country column index
    std::string header;
    if (!std::getline(file, header)) {
        std::cerr << "Error: Could not read header from file." << std::endl;
        return data;
    }
    
    // Find the column index for the specified country
    std::string targetColumn = countryCode + "_temperature";
    int countryColumnIndex = -1;
    int currentIndex = 0;
    
    std::stringstream headerStream(header);
    std::string column;
    
    while (std::getline(headerStream, column, ',')) {
        // Remove any whitespace from column name
        column.erase(0, column.find_first_not_of(" \t"));
        column.erase(column.find_last_not_of(" \t") + 1);
        
        if (column == targetColumn) {
            countryColumnIndex = currentIndex;
            break;
        }
        currentIndex++;
    }
    
    // Check if country column was found
    if (countryColumnIndex == -1) {
        std::cerr << "Error: Country '" << countryCode << "' not found in CSV." << std::endl;
        std::cerr << "Looking for column: '" << targetColumn << "'" << std::endl;
        std::cerr << "Available columns in header: " << header << std::endl;
        return data;
    }
    
    // Process each data row
    std::string line;
    int lineNumber = 1; // Start at 1 since header is line 0
    int recordsProcessed = 0;
    int recordsInRange = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        recordsProcessed++;
        
        if (line.empty()) {
            continue; // Skip empty lines
        }
        
        // Parse the line by splitting on commas
        std::vector<std::string> columns;
        std::stringstream lineStream(line);
        std::string cell;
        
        while (std::getline(lineStream, cell, ',')) {
            columns.push_back(cell);
        }
        
        // Check if we have enough columns
        if (columns.size() <= static_cast<size_t>(countryColumnIndex)) {
            std::cerr << "Warning: Line " << lineNumber << " has insufficient columns. Skipping." << std::endl;
            continue;
        }
        
        // Extract date from first column
        std::string dateStr = columns[0];
        
        // Remove any whitespace from date
        dateStr.erase(0, dateStr.find_first_not_of(" \t"));
        dateStr.erase(dateStr.find_last_not_of(" \t") + 1);
        
        // Extract year from date string (first 4 characters)
        if (dateStr.length() < 4) {
            std::cerr << "Warning: Invalid date format on line " << lineNumber << ": '" << dateStr << "'. Skipping." << std::endl;
            continue;
        }
        
        std::string yearStr = dateStr.substr(0, 4);
        int year;
        
        try {
            year = std::stoi(yearStr);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not parse year from '" << yearStr << "' on line " << lineNumber << ". Skipping." << std::endl;
            continue;
        }
        
        // Check if year is within specified range
        if (year < startYear || year > endYear) {
            continue; // Skip records outside date range
        }
        
        recordsInRange++;
        
        // Extract temperature value from country column
        std::string tempStr = columns[countryColumnIndex];
        
        // Remove any whitespace from temperature string
        tempStr.erase(0, tempStr.find_first_not_of(" \t"));
        tempStr.erase(tempStr.find_last_not_of(" \t") + 1);
        
        // Handle empty temperature values
        if (tempStr.empty() || tempStr == "NA" || tempStr == "N/A" || tempStr == "-") {
            continue; // Skip records with missing temperature data
        }
        
        double temperature;
        try {
            temperature = std::stod(tempStr);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not parse temperature '" << tempStr << "' on line " << lineNumber << ". Skipping." << std::endl;
            continue;
        }
        
        // Create and store the temperature record
        TemperatureRecord record;
        record.date = dateStr;
        record.temperature = temperature;
        data.push_back(record);
    }
    
    file.close();
    
    return data;
}