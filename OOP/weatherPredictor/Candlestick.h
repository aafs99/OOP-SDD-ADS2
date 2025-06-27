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
    
    // 
    /**
     *  Gets the mean temperature for this period
     * @return The average temperature 
     * @note Inthe temperature candlestick model, 'close' represents 
     *       the calculated average temperature for the time period
     */
    double getMeanTemperature() const { return close_; }
    
    // Legacy method for backward compatibility
    [[deprecated("Use getMeanTemperature() instead for clarity")]]
    double getAverage() const { return close_; }
};

#endif