#pragma once

#include "types.hpp"
#include <zmq.hpp>
#include <string>
#include <iostream>

namespace ullt {

class MarketDataPublisher {
private:
    zmq::context_t context;
    zmq::socket_t publisher;

public:
    explicit MarketDataPublisher(const std::string& bind_addr = "tcp://*:5555") 
        : context(1), publisher(context, zmq::socket_type::pub) {
        publisher.bind(bind_addr);
        std::cout << "Market Data Publisher bound to " << bind_addr << "\n";
    }

    // High throughput zero-copy or direct cast publish
    void publish_trade(const Trade& trade) {
        // Prepare ZMQ message holding raw struct
        zmq::message_t msg(sizeof(Trade));
        memcpy(msg.data(), &trade, sizeof(Trade));
        
        // Topic "TRADE_" could be prepended if we multiplex, but for simplicity we just dump the struct
        publisher.send(msg, zmq::send_flags::none);
    }
};

} // namespace ullt
