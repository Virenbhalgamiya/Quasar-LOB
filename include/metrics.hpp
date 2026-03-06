#pragma once

#include "types.hpp"
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>

namespace ullt {

class Metrics {
private:
    std::vector<uint64_t> latencies_ns; // Round trip latency per order
    uint64_t total_orders{0};
    uint64_t start_time_ns{0};

public:
    Metrics() {
        latencies_ns.reserve(1000000); // Reserve memory for 1M orders to avoid reallocation spikes
    }

    void start_timer() {
        start_time_ns = current_time_ns();
    }

    void record_latency(uint64_t order_creation_ns) {
        uint64_t now = current_time_ns();
        if (now > order_creation_ns) {
            latencies_ns.push_back(now - order_creation_ns);
        }
        total_orders++;
    }

    void print_report() {
        if (latencies_ns.empty()) {
            std::cout << "No latencies recorded.\n";
            return;
        }

        std::sort(latencies_ns.begin(), latencies_ns.end());

        uint64_t elapsed_ns = current_time_ns() - start_time_ns;
        double elapsed_sec = static_cast<double>(elapsed_ns) / 1e9;
        double throughput = static_cast<double>(total_orders) / elapsed_sec;

        uint64_t sum = std::accumulate(latencies_ns.begin(), latencies_ns.end(), 0ULL);
        uint64_t mean = sum / latencies_ns.size();
        
        uint64_t p50 = latencies_ns[latencies_ns.size() * 0.50];
        uint64_t p90 = latencies_ns[latencies_ns.size() * 0.90];
        uint64_t p99 = latencies_ns[latencies_ns.size() * 0.99];
        uint64_t max_lat = latencies_ns.back();

        std::cout << "--- Performance Report ---\n";
        std::cout << "Total Orders: " << total_orders << "\n";
        std::cout << "Elapsed Time: " << elapsed_sec << " s\n";
        std::cout << "Throughput:   " << static_cast<uint64_t>(throughput) << " ops/sec\n";
        std::cout << "Latency (ns):\n";
        std::cout << "  Mean: " << mean << " ns\n";
        std::cout << "  p50:  " << p50 << " ns\n";
        std::cout << "  p90:  " << p90 << " ns\n";
        std::cout << "  p99:  " << p99 << " ns\n";
        std::cout << "  Max:  " << max_lat << " ns\n";
        std::cout << "--------------------------\n";
    }
};

} // namespace ullt
