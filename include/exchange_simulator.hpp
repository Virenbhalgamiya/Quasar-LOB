#pragma once

#include "matching_engine.hpp"
#include "market_data_publisher.hpp"
#include <thread>
#include <atomic>
#include <random>

namespace ullt {

class ExchangeSimulator {
private:
    MatchingEngine engine;
    MarketDataPublisher mdp;
    std::atomic<bool> running{false};
    std::thread sim_thread;

public:
    ExchangeSimulator() : mdp("tcp://*:5555") {}

    void start() {
        running = true;
        sim_thread = std::thread([this]() {
            run_loop();
        });
        std::cout << "Exchange Simulator started.\n";
    }

    void stop() {
        running = false;
        if (sim_thread.joinable()) {
            sim_thread.join();
        }
    }

private:
    void run_loop() {
        // Random order generator
        std::random_device rd;
        std::mt19random mt(rd());
        std::uniform_int_distribution<Price> price_dist(9900, 10100);
        std::uniform_int_distribution<Quantity> qty_dist(10, 100);
        std::uniform_int_distribution<int> side_dist(0, 1);

        size_t trade_index = 0;

        while (running) {
            Price p = price_dist(mt);
            Quantity q = qty_dist(mt);
            Side s = side_dist(mt) == 0 ? Side::Buy : Side::Sell;

            // Place order into matching engine
            engine.place_order(p, q, s, OrderType::Limit);

            // Fetch new trades generated and publish them
            const auto& trades = engine.get_trades();
            while (trade_index < trades.size()) {
                mdp.publish_trade(trades[trade_index]);
                trade_index++;
            }

            // Sleep sparingly to simulate steady tick flow, or let it blast
            // We'll sleep to roughly 10k orders/sec for stability during dev
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
};

} // namespace ullt
