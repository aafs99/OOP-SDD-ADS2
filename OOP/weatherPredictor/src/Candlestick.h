#ifndef CANDLESTICK_H
#define CANDLESTICK_H

#include <string>

class Candlestick {
private:
    std::string date_;
    double open_;
    double close_;
    double high_;
    double low_;

public:
    Candlestick();
    Candlestick(const std::string& date, double open, double close, double high, double low);

    // Getters
    std::string getDate() const { return date_; }
    double getOpen() const { return open_; }
    double getClose() const { return close_; }
    double getHigh() const { return high_; }
    double getLow() const { return low_; }
    
    // Utility methods
    bool isUptrend() const { return close_ >= open_; }
    double getVolatility() const { return high_ - low_; }
    double getAverage() const { return close_; } // Using close as average temperature
};

#endif