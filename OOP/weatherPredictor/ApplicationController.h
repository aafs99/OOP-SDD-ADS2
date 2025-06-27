#ifndef APPLICATION_CONTROLLER_H
#define APPLICATION_CONTROLLER_H

#include <vector>
#include <string>
#include "Common.h"
#include "Candlestick.h"
#include "TemperatureRecord.h"

/**
 *  Main application controller that manages the analysis workflow
 * 
 * This class encapsulates the main business logic,
 * separating it from the main() function for better organization and testability.
 */
class ApplicationController {
public:
    /**
     *  Constructs the application controller with data file path
     * @param dataFilePath Path to the CSV data file
     */
    explicit ApplicationController(const std::string& dataFilePath);
    
    /**
     *  Runs the main application loop
     * @return Exit code (0 for success, non-zero for error)
     */
    int run();

private:
    // Core workflow methods
    bool performSingleAnalysis();
    bool loadAndProcessData(const std::string& country, int startYear, int endYear, 
                           TimeFrame timeframe, std::vector<Candlestick>& candlesticks);
    void displayInitialResults(const std::vector<Candlestick>& candlesticks, 
                              TimeFrame timeframe, const std::string& country,
                              int startYear, int endYear);
    bool handleFilteringWorkflow(std::vector<Candlestick>& dataForAnalysis,
                                const std::vector<Candlestick>& originalData,
                                TimeFrame timeframe, const std::string& country);
    void handlePredictionWorkflow(const std::vector<Candlestick>& dataForAnalysis,
                                 const std::vector<Candlestick>& originalData,
                                 TimeFrame timeframe, const std::string& country,
                                 int startYear, int endYear, bool wasFiltered);
    
    // Utility methods
    void showWelcomeMessage();
    void showFarewellMessage();
    bool handleException(const std::exception& e, const std::string& context);
    
    // Member variables
    std::string dataFilePath_;
    
    // Static helper methods
    static std::vector<Candlestick> applyFilter(
        std::vector<Candlestick> data,
        FilterType filterType,
        const std::string& startDate,
        const std::string& endDate,
        double minTemp,
        double maxTemp,
        bool uptrend,
        double minVolatility
    );
    
    static void displayChartAndTable(const std::vector<Candlestick>& data, 
                                   TimeFrame timeframe, 
                                   const std::string& country, 
                                   const std::string& chartTitle = "");
    
    static void displayPredictions(const std::vector<Candlestick>& data, 
                                 TimeFrame timeframe, 
                                 const std::string& country, 
                                 int startYear, int endYear);
};

#endif // APPLICATION_CONTROLLER_H