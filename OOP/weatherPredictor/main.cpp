#include <iostream>
#include <string>
#include "ApplicationController.h"


//self written code start
/**
 * 
 * This program provides a command-line interface for analyzing historical
 * temperature data from European countries (1980-2019). It supports:
 * 
 * - Candlestick data computation and visualization
 * - Text-based plotting with intelligent compression
 * - Advanced filtering (date, temperature, trend, volatility)
 * - Temperature prediction using multiple models with confidence metrics
 * - Cross-validation and prediction comparison charts
 * 
 * @return 0 on successful completion, non-zero on error
 */
int main() {
    // Configuration
    const std::string dataFilePath = "./weather_data_EU_1980-2019_temp_only.csv";
    
    try {
        // Create and run the application controller
        ApplicationController app(dataFilePath);
        return app.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        std::cerr << "The application cannot continue and will now exit." << std::endl;
        return 1;
        
    } catch (...) {
        std::cerr << "Fatal Error: Unknown system error occurred." << std::endl;
        std::cerr << "The application cannot continue and will now exit." << std::endl;
        return 2;
    }
}

//self written code end