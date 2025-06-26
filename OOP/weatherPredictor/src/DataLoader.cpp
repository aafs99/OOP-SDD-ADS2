#include "DataLoader.h"
#include "Common.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace DataLoader {

std::vector<TemperatureRecord> loadCSV(const std::string& filename, const std::string& countryCode, 
                                       int startYear, int endYear) {
    std::vector<TemperatureRecord> data;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
        std::cerr << "Please ensure the file exists in the same directory as the executable." << std::endl;
        return data;
    }
    
    std::string header;
    if (!std::getline(file, header)) {
        std::cerr << "Error: Could not read header from file." << std::endl;
        return data;
    }
    
    std::string targetColumn = countryCode + "_temperature";
    int countryColumnIndex = -1;
    int currentIndex = 0;
    
    std::stringstream headerStream(header);
    std::string column;
    
    while (std::getline(headerStream, column, ',')) {
        column.erase(0, column.find_first_not_of(" \t"));
        column.erase(column.find_last_not_of(" \t") + 1);
        
        if (column == targetColumn) {
            countryColumnIndex = currentIndex;
            break;
        }
        currentIndex++;
    }
    
    if (countryColumnIndex == -1) {
        std::cerr << "Error: Country '" << countryCode << "' not found in CSV." << std::endl;
        std::cerr << "Looking for column: '" << targetColumn << "'" << std::endl;
        std::cerr << "Available columns in header: " << header << std::endl;
        return data;
    }
    
    std::string line;
    int lineNumber = 1;
    
    while (std::getline(file, line)) {
        lineNumber++;
        
        if (line.empty()) continue;
        
        std::vector<std::string> columns;
        std::stringstream lineStream(line);
        std::string cell;
        
        while (std::getline(lineStream, cell, ',')) {
            columns.push_back(cell);
        }
        
        if (columns.size() <= static_cast<size_t>(countryColumnIndex)) {
            std::cerr << "Warning: Line " << lineNumber << " has insufficient columns. Skipping." << std::endl;
            continue;
        }
        
        std::string dateStr = columns[0];
        dateStr.erase(0, dateStr.find_first_not_of(" \t"));
        dateStr.erase(dateStr.find_last_not_of(" \t") + 1);
        
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
        
        if (year < startYear || year > endYear) continue;
        
        std::string tempStr = columns[countryColumnIndex];
        tempStr.erase(0, tempStr.find_first_not_of(" \t"));
        tempStr.erase(tempStr.find_last_not_of(" \t") + 1);
        
        if (tempStr.empty() || tempStr == "NA" || tempStr == "N/A" || tempStr == "-") {
            continue;
        }
        
        double temperature;
        try {
            temperature = std::stod(tempStr);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not parse temperature '" << tempStr << "' on line " << lineNumber << ". Skipping." << std::endl;
            continue;
        }
        
        data.push_back(TemperatureRecord(dateStr, temperature));
    }
    
    file.close();
    return data;
}

} // namespace DataLoader