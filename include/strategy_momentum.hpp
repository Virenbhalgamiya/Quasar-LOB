#pragma once

#include "strategy.hpp"
#include <vector>
#include <iostream>

namespace ullt {

class MomentumStrategy : public Strategy {
private:
    std::vector<Price> price_history;
    size_t period;

public:
    explicit MomentumStrategy(size_t p = 10) : period(p) {}

    void on_trade(const Trade& trade) override {
        price_history.push_back(trade.price);
        if (price_history.size() > period) {
            price_history.erase(price_history.begin());
        }

        if (price_history.size() == period) {
            Price old_price = price_history.front();
            Price current_price = price_history.back();

            if (current_price > old_price + 10) {
                // Signal Buy
                std::cout << "[Strategy] Momentum Buy Signal at " << current_price << "\n";
            } else if (current_price < old_price - 10) {
                // Signal Sell
                std::cout << "[Strategy] Momentum Sell Signal at " << current_price << "\n";
            }
        }
    }

    void on_tick() override {
        // Evaluate non-trade based signals
    }
};

} // namespace ullt
