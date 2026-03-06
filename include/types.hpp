#pragma once

#include <cstdint>
#include <chrono>

namespace ullt {

enum class Side : uint8_t {
    Buy = 1,
    Sell = 2
};

enum class OrderType : uint8_t {
    Limit = 1,
    Market = 2
};

enum class OrderStatus : uint8_t {
    New = 1,
    PartiallyFilled = 2,
    Filled = 3,
    Canceled = 4,
    Rejected = 5
};

using OrderId = uint64_t;
using Price = uint32_t;  // Prices can be represented as ticks/integer (e.g. 100.50 -> 10050)
using Quantity = uint32_t;

// Cache-line aligned Order structure
// Keeping this small helps it fit into CPU cache easily.
struct alignas(64) Order {
    OrderId id;
    Price price;
    Quantity initial_qty;
    Quantity remaining_qty;
    Side side;
    OrderType type;
    OrderStatus status;
    uint64_t timestamp_ns;

    Order* next{nullptr};
    Order* prev{nullptr};
};

struct Trade {
    OrderId maker_order_id;
    OrderId taker_order_id;
    Price price;
    Quantity qty;
    uint64_t timestamp_ns;
};

// Helper for high-res time
inline uint64_t current_time_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
}

} // namespace ullt
