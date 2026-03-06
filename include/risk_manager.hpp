#pragma once

#include "types.hpp"
#include <atomic>
#include <chrono>

namespace ullt {

class RiskManager {
private:
    std::atomic<int32_t> position{0};
    int32_t max_position;

    // Token bucket for rate limiting
    std::atomic<uint32_t> tokens;
    uint32_t max_tokens;
    uint32_t refill_rate_per_sec;
    uint64_t last_refill_ns;

public:
    explicit RiskManager(int32_t max_pos = 1000, uint32_t rate_limit = 10000)
        : max_position(max_pos), max_tokens(rate_limit), refill_rate_per_sec(rate_limit) {
        tokens.store(rate_limit);
        last_refill_ns = current_time_ns();
    }

    bool check_order(Side side, Quantity qty, Price /*price*/) {
        // 1. Rate Check (Token Bucket)
        refill_tokens();
        uint32_t current_tokens = tokens.load(std::memory_order_acquire);
        if (current_tokens == 0) [[unlikely]] {
            return false; // Order rate exceeded
        }

        // 2. Position Check
        int32_t pos = position.load(std::memory_order_acquire);
        int32_t projected_pos = pos + (side == Side::Buy ? qty : -static_cast<int32_t>(qty));
        
        if (projected_pos > max_position || projected_pos < -max_position) [[unlikely]] {
            return false; // Position limit exceeded
        }

        // Consume token (simplified atomic decrement, in reality needs CAS loop for precision)
        // For sub-microsecond latency, optimistic atomic decrement is used
        tokens.fetch_sub(1, std::memory_order_release);
        return true;
    }

    // Called async when trades are executed to update actual position
    void on_trade_executed(Side side, Quantity qty) {
        if (side == Side::Buy) {
            position.fetch_add(qty, std::memory_order_relaxed);
        } else {
            position.fetch_sub(qty, std::memory_order_relaxed);
        }
    }

private:
    void refill_tokens() {
        uint64_t now = current_time_ns();
        uint64_t elapsed = now - last_refill_ns;
        // 1 sec = 1,000,000,000 ns
        if (elapsed > 1000000000ULL / refill_rate_per_sec) {
            uint32_t add = (elapsed * refill_rate_per_sec) / 1000000000ULL;
            if (add > 0) {
                uint32_t current = tokens.load(std::memory_order_relaxed);
                uint32_t new_val = std::min(current + add, max_tokens);
                if (tokens.compare_exchange_weak(current, new_val)) {
                    last_refill_ns = now;
                }
            }
        }
    }
};

} // namespace ullt
