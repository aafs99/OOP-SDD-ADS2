#ifndef PLOT_CONFIGURATION_H
#define PLOT_CONFIGURATION_H

#include <string>

struct PlotConfiguration {
    int candleWidth;
    int candleSpacing;
    std::string upTrendBody;
    std::string downTrendBody;
    std::string wickShape;
    std::string emptySpace;
    std::string spacingStr;

    PlotConfiguration(int width = 5, int spacing = 2);
    void update();
};

// Compression levels for different display densities
struct CompressionLevel {
    int candleWidth;
    int candleSpacing;
    std::string description;
};

#endif