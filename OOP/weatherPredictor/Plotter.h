#ifndef PLOTTER_H
#define PLOTTER_H

#include <vector>
#include <string>
#include "Candlestick.h"
#include "Common.h"
#include "PlotConfiguration.h"

namespace Plotter {
    void plotCandlesticks(
        const std::vector<Candlestick>& candlesticks, 
        TimeFrame timeframe, 
        int chartHeight = 20
    );
    
    namespace Internal {
        std::vector<Candlestick> applyIntelligentSampling(
            const std::vector<Candlestick>& candlesticks, 
            TimeFrame timeframe
        );
        
        PlotConfiguration determineOptimalCompression(
            size_t numCandlesticks, 
            TimeFrame timeframe
        );
        
        void printChartHeader(
            const PlotConfiguration& config, 
            bool wasDownsampled, 
            size_t originalSize, 
            size_t displaySize
        );
        
        void printXAxisLabels(
            const std::vector<Candlestick>& candlesticks, 
            TimeFrame timeframe, 
            const PlotConfiguration& config
        );
        
        std::string formatDateLabel(
            const std::string& date, 
            TimeFrame timeframe, 
            int maxWidth
        );
        
        std::string getCandlestickAtRow(
            const Candlestick& candle, 
            double rowTemp, 
            double tempPerRow, 
            const PlotConfiguration& config
        );
        
        double findMinTemperature(const std::vector<Candlestick>& candlesticks);
        double findMaxTemperature(const std::vector<Candlestick>& candlesticks);
        void printSummary(const std::vector<Candlestick>& candlesticks);
    }
}

#endif