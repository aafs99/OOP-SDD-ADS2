#include "DataLoader.h"
#include "Common.h"
#include <fstream>
#include <sstream>
#include <iostream>

/*self written code start (code built with reference to : 
 https://github.com/KC1922/StockMarketDisplay-CPP/tree/main/StockProjectCPP
 https://github.com/AriaFallah/csv-parser
 https://github.com/tdulcet/Table-and-Graph-Libs
 https://www.youtube.com/watch?v=Ox7qVlfNYNE&list=PLiPFKCVZWHKqk1TSSQTBcPSxZEjBT9E88&index=6
*/
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
    
    // ENHANCED: More robust column name matching
    std::string targetColumn = countryCode + "_temperature";
    int countryColumnIndex = -1;
    int currentIndex = 0;
    
    std::stringstream headerStream(header);
    std::string column;
    
    while (std::getline(headerStream, column, ',')) {
        // Remove quotes if present and trim whitespace
        column.erase(0, column.find_first_not_of(" \t\""));
        column.erase(column.find_last_not_of(" \t\"") + 1);
        
        if (column == targetColumn) {
            countryColumnIndex = currentIndex;
            break;
        }
        currentIndex++;
    }
    
    if (countryColumnIndex == -1) {
        std::cerr << "Error: Country '" << countryCode << "' not found in CSV." << std::endl;
        std::cerr << "Looking for column: '" << targetColumn << "'" << std::endl;
        
        // ENHANCED: Show available country columns to help user
        std::cerr << "Available country columns: ";
        std::stringstream tempHeaderStream(header);
        std::string tempColumn;
        bool foundCountryColumns = false;
        while (std::getline(tempHeaderStream, tempColumn, ',')) {
            tempColumn.erase(0, tempColumn.find_first_not_of(" \t\""));
            tempColumn.erase(tempColumn.find_last_not_of(" \t\"") + 1);
            if (tempColumn.length() == 2 || tempColumn.find("_temperature") != std::string::npos) {
                if (foundCountryColumns) std::cerr << ", ";
                std::cerr << tempColumn;
                foundCountryColumns = true;
            }
        }
        std::cerr << std::endl;
        return data;
    }
    
    std::string line;
    int lineNumber = 1;
    int validRecords = 0;
    int skippedRecords = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        
        if (line.empty()) continue;
        
        std::vector<std::string> columns;
        std::stringstream lineStream(line);
        std::string cell;
        
        // ENHANCED: Better CSV parsing to handle quoted values
        bool inQuotes = false;
        std::string currentCell;
        
        for (char c : line) {
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (c == ',' && !inQuotes) {
                columns.push_back(currentCell);
                currentCell.clear();
            } else {
                currentCell += c;
            }
        }
        columns.push_back(currentCell); // Add the last cell
        
        if (columns.size() <= static_cast<size_t>(countryColumnIndex)) {
            skippedRecords++;
            continue;
        }
        
        std::string dateStr = columns[0];
        dateStr.erase(0, dateStr.find_first_not_of(" \t\""));
        dateStr.erase(dateStr.find_last_not_of(" \t\"") + 1);
        
        if (dateStr.length() < 4) {
            skippedRecords++;
            continue;
        }
        
        std::string yearStr = dateStr.substr(0, 4);
        int year;
        
        try {
            year = std::stoi(yearStr);
        } catch (const std::exception& e) {
            skippedRecords++;
            continue;
        }
        
        if (year < startYear || year > endYear) continue;
        
        std::string tempStr = columns[countryColumnIndex];
        tempStr.erase(0, tempStr.find_first_not_of(" \t\""));
        tempStr.erase(tempStr.find_last_not_of(" \t\"") + 1);
        
        // ENHANCED: More comprehensive handling of missing values
        if (tempStr.empty() || tempStr == "NA" || tempStr == "N/A" || 
            tempStr == "-" || tempStr == "null" || tempStr == "NULL" ||
            tempStr == "nan" || tempStr == "NaN") {
            continue;
        }
        
        double temperature;
        try {
            temperature = std::stod(tempStr);
            
            // ENHANCED: Sanity check for temperature values (reasonable range for Earth)
            if (temperature < -100.0 || temperature > 100.0) {
                std::cerr << "Warning: Unusual temperature value " << temperature 
                          << "°C on line " << lineNumber << ". Skipping." << std::endl;
                skippedRecords++;
                continue;
            }
            
        } catch (const std::exception& e) {
            skippedRecords++;
            continue;
        }
        
        data.push_back(TemperatureRecord(dateStr, temperature));
        validRecords++;
    }
    
    file.close();
    
    // ENHANCED: Provide summary of data loading
    std::cout << "Data loading summary:" << std::endl;
    std::cout << "  Valid records loaded: " << validRecords << std::endl;
    if (skippedRecords > 0) {
        std::cout << "  Records skipped: " << skippedRecords << std::endl;
    }
    
    return data;
}

} // namespace DataLoader
//self written code end