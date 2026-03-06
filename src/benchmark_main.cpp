#include "matching_engine.hpp"
#include "risk_manager.hpp"
#include "metrics.hpp"
#include <iostream>
#include <vector>

using namespace ullt;

void run_stress_test(size_t order_count) {
    RiskManager rm(1000000, 10000000); // Virtually infinite risk limits to test pure engine perf
    MatchingEngine engine(order_count + 100);
    Metrics metrics;

    std::cout << "Initializing Stress Test with " << order_count << " orders...\n";

    // Pre-populate LOB with resting orders to simulate real thick book
    for (size_t i = 0; i < 1000; i++) {
        engine.place_order(10000 - i, 10, Side::Buy, OrderType::Limit); // Bids
        engine.place_order(10050 + i, 10, Side::Sell, OrderType::Limit); // Asks
    }

    metrics.start_timer();

    // Blast orders into the engine
    for (size_t i = 0; i < order_count; i++) {
        uint64_t order_ts = current_time_ns();
        
        bool approved = rm.check_order(Side::Buy, 10, 10025);
        if (approved) {
            engine.place_order(10025, 10, Side::Buy, OrderType::Limit);
            metrics.record_latency(order_ts); // Measure latency of creation -> risk check -> map -> engine placement
        }
    }

    metrics.print_report();
}

int main() {
    run_stress_test(1000000); // 1 Million Orders
    return 0;
}
