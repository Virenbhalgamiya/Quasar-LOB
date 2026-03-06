#include "matching_engine.hpp"
#include "exchange_simulator.hpp"
#include "risk_manager.hpp"
#include "strategy_momentum.hpp"
#include <iostream>
#include <chrono>

using namespace ullt;

int main() {
    std::cout << "Ultra Low Latency Trading System Starting...\n";

    // 1. Initialize core components
    RiskManager risk_manager(1000, 5000); // Max position 1000, 5000 orders/sec rate limit
    
    // 2. Initialize simulator (Matching Engine + Market Data Publisher)
    ExchangeSimulator simulator;
    
    // 3. Initialize test strategy
    MomentumStrategy strategy(5);

    // Provide a simple loop to demonstrate functionality locally
    // In a real system, the Strategy would be listening on a ZeroMQ SUB socket 
    // to the MarketDataPublisher. Here we just drive it synthetically.
    
    simulator.start();

    auto start_time = std::chrono::steady_clock::now();
    size_t simulated_orders = 0;

    // Simulate 100 ms of vigorous trading
    while (std::chrono::steady_clock::now() - start_time < std::chrono::milliseconds(100)) {
        // Assume strategy wants to buy
        if (risk_manager.check_order(Side::Buy, 10, 10000)) {
            // Internally dispatched to Order Manager -> Matching Engine
            risk_manager.on_trade_executed(Side::Buy, 10);
            simulated_orders++;
        }
    }

    simulator.stop();

    std::cout << "Simulation completed.\n";
    std::cout << "Orders past risk manager in 100ms: " << simulated_orders << "\n";
    std::cout << "Estimated throughput: " << simulated_orders * 10 << " orders/sec (rate limited by config)\n";

    return 0;
}
