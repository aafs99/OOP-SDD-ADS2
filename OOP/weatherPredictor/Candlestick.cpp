#include "Candlestick.h"

Candlestick::Candlestick() : date_(""), open_(0), close_(0), high_(0), low_(0) {}

Candlestick::Candlestick(const std::string& date, double open, double close, double high, double low)
    : date_(date), open_(open), close_(close), high_(high), low_(low) {}