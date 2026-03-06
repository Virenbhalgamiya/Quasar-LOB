#pragma once

#include "types.hpp"

namespace ullt {

class Strategy {
public:
    virtual ~Strategy() = default;

    // Called when market data is received
    virtual void on_trade(const Trade& trade) = 0;

    // Called on internal timer tick or external event
    virtual void on_tick() = 0;

    // The strategy is expected to carry a reference to a TradingGateway/RiskManager 
    // to place orders when conditions are met.
};

} // namespace ullt
