#pragma once

#include "strategy.hpp"
#include <iostream>

namespace ullt {

class MarketMakingStrategy : public Strategy {
private:
    Price fair_value;
    Price spread;
    Quantity clip_size;

public:
    explicit MarketMakingStrategy(Price fv = 10000, Price s = 10, Quantity qty = 100) 
        : fair_value(fv), spread(s), clip_size(qty) {}

    void on_trade(const Trade& trade) override {
        // Update fair value based on last trade (simplified micro-price model)
        fair_value = (fair_value * 9 + trade.price) / 10;
    }

    void on_tick() override {
        // Periodically quote both sides
        Price bid = fair_value - (spread / 2);
        Price ask = fair_value + (spread / 2);
        
        std::cout << "[Strategy] MM Quoting: Bid " << clip_size << " @ " << bid 
                  << " | Ask " << clip_size << " @ " << ask << "\n";
    }
};

} // namespace ullt
