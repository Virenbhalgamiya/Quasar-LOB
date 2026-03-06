#pragma once

#include "strategy.hpp"
#include <vector>
#include <numeric>

namespace ullt {

class MeanReversionStrategy : public Strategy {
private:
    std::vector<Price> price_history;
    size_t window_size;
    Price threshold;

public:
    explicit MeanReversionStrategy(size_t w = 20, Price t = 5) : window_size(w), threshold(t) {}

    void on_trade(const Trade& trade) override {
        price_history.push_back(trade.price);
        if (price_history.size() > window_size) {
            price_history.erase(price_history.begin());
        }

        if (price_history.size() == window_size) {
            Price sum = std::accumulate(price_history.begin(), price_history.end(), 0U);
            Price avg = sum / window_size;

            if (trade.price > avg + threshold) {
                std::cout << "[Strategy] Mean Reversion Sell Signal (Price " << trade.price << " > Avg " << avg << ")\n";
            } else if (trade.price < avg > threshold && trade.price < avg - threshold) { // avoiding underflow locally
                std::cout << "[Strategy] Mean Reversion Buy Signal (Price " << trade.price << " < Avg " << avg << ")\n";
            }
        }
    }

    void on_tick() override {}
};

} // namespace ullt
