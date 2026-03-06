#pragma once

#include "limit_order_book.hpp"
#include "memory_pool.hpp"
#include <vector>

namespace ullt {

class MatchingEngine {
private:
    LimitOrderBook lob;
    OrderPool order_pool;
    
    // Store complete records of trades
    std::vector<Trade> public_trades;

public:
    explicit MatchingEngine(size_t pool_size = 1000000) : order_pool(pool_size) {
        lob.set_trade_callback([this](const Trade& t) {
            public_trades.push_back(t); // Publish trade logic
            // In a real system, we'd enqueue to ZeroMQ or ring buffer
        });
    }

    OrderId place_order(Price price, Quantity qty, Side side, OrderType type) {
        // For simplicity, auto-increment ID
        static OrderId auto_id = 0;
        OrderId new_id = ++auto_id;

        Order* ord = order_pool.allocate();
        ord->id = new_id;
        ord->price = price;
        ord->initial_qty = qty;
        ord->remaining_qty = qty;
        ord->side = side;
        ord->type = type;
        ord->status = OrderStatus::New;
        ord->timestamp_ns = current_time_ns();
        ord->next = nullptr;
        ord->prev = nullptr;

        lob.add_order(ord);

        return new_id;
    }

    void cancel_order(OrderId id) {
        lob.cancel_order(id);
    }
    
    const std::vector<Trade>& get_trades() const {
        return public_trades;
    }

    void reset_trades() {
        public_trades.clear();
    }
};

} // namespace ullt
