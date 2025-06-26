#include "PlotConfiguration.h"

PlotConfiguration::PlotConfiguration(int width, int spacing) 
    : candleWidth(width), candleSpacing(spacing) {
    update();
}

void PlotConfiguration::update() {
    upTrendBody = std::string(candleWidth, '+');
    downTrendBody = std::string(candleWidth, '-');
    emptySpace = std::string(candleWidth, ' ');
    spacingStr = std::string(candleSpacing, ' ');

    if (candleWidth > 0) {
        wickShape = std::string(candleWidth, ' ');
        wickShape[candleWidth / 2] = '|';
    } else {
        wickShape = "|";
    }
}