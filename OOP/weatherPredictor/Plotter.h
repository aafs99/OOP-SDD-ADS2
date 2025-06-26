#ifndef PLOTTER_H
#define PLOTTER_H

#include <vector>
#include <string>
#include "Candlestick.h"
#include "Common.h"
#include "PlotConfiguration.h"

namespace Plotter {
    /**
     * @brief Plots a vector of Candlestick data to the console.
     * * This function handles the entire process of rendering the chart, including
     * intelligent sampling of data, optimal compression for display, and printing
     * the Y-axis, X-axis, and a summary.
     * * @param candlesticks The vector of Candlestick objects to plot.
     * @param timeframe The timeframe (Daily, Monthly, Yearly) of the data.
     * @param chartHeight The desired height of the chart in character rows.
     */
    void plotCandlesticks(
        const std::vector<Candlestick>& candlesticks, 
        TimeFrame timeframe, 
        int chartHeight = 20
    );
}

#endif