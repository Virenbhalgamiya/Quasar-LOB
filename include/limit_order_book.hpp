#pragma once

#include "types.hpp"
#include <map>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <functional>

namespace ullt {

// A price level holds a queue of orders
class PriceLevel {
private:
    Order* head{nullptr};
    Order* tail{nullptr};
    Quantity total_volume{0};

public:
    void append(Order* order) {
        if (!head) {
            head = tail = order;
        } else {
            tail->next = order;
            order->prev = tail;
            tail = order;
        }
        total_volume += order->remaining_qty;
    }

    void remove(Order* order) {
        if (order->prev) {
            order->prev->next = order->next;
        } else if (head == order) {
            head = order->next;
        }

        if (order->next) {
            order->next->prev = order->prev;
        } else if (tail == order) {
            tail = order->prev;
        }

        total_volume -= order->remaining_qty;
        order->next = nullptr;
        order->prev = nullptr;
    }

    Order* get_head() const { return head; }
    Quantity get_volume() const { return total_volume; }
    bool is_empty() const { return head == nullptr; }

    // Modify volume without unlinking (partial fills)
    void reduce_volume(Quantity qty) {
        total_volume -= qty;
    }
};

class LimitOrderBook {
private:
    // For bids, highest price first (std::greater)
    std::map<Price, PriceLevel, std::greater<Price>> bids;
    // For asks, lowest price first (std::less)
    std::map<Price, PriceLevel, std::less<Price>> asks;

    // Fast order lookup by ID
    std::unordered_map<OrderId, Order*> order_map;

    using TradeCallback = std::function<void(const Trade&)>;
    TradeCallback on_trade;

public:
    LimitOrderBook() = default;

    void set_trade_callback(TradeCallback cb) {
        on_trade = std::move(cb);
    }

    void add_order(Order* order) {
        // First try to match directly
        match_order(order);

        // If order hasn't been completely filled, rest it in the book
        if (order->remaining_qty > 0 && order->type == OrderType::Limit) {
            if (order->side == Side::Buy) {
                bids[order->price].append(order);
            } else {
                asks[order->price].append(order);
            }
            order->status = OrderStatus::New; // or PartiallyFilled
            order_map[order->id] = order;
        } else if (order->remaining_qty > 0 && order->type == OrderType::Market) {
            // Unfilled market orders are usually cancelled
            order->status = OrderStatus::Canceled;
        }
    }

    void cancel_order(OrderId id) {
        auto it = order_map.find(id);
        if (it != order_map.end()) {
            Order* order = it->second;
            if (order->side == Side::Buy) {
                bids[order->price].remove(order);
                if (bids[order->price].is_empty()) {
                    bids.erase(order->price);
                }
            } else {
                asks[order->price].remove(order);
                if (asks[order->price].is_empty()) {
                    asks.erase(order->price);
                }
            }
            order->status = OrderStatus::Canceled;
            order_map.erase(it);
        }
    }

private:
    void match_order(Order* incoming) {
        if (incoming->side == Side::Buy) {
            auto it = asks.begin();
            while (it != asks.end() && incoming->remaining_qty > 0) {
                Price best_ask_price = it->first;
                
                // If limit order, check if we can cross
                if (incoming->type == OrderType::Limit && incoming->price < best_ask_price) {
                    break;
                }

                PriceLevel& level = it->second;
                execute_against_level(incoming, level, Side::Sell);

                if (level.is_empty()) {
                    it = asks.erase(it);
                } else {
                    break; // incoming fully filled
                }
            }
        } else { // Sell order
            auto it = bids.begin();
            while (it != bids.end() && incoming->remaining_qty > 0) {
                Price best_bid_price = it->first;

                if (incoming->type == OrderType::Limit && incoming->price > best_bid_price) {
                    break;
                }

                PriceLevel& level = it->second;
                execute_against_level(incoming, level, Side::Buy);

                if (level.is_empty()) {
                    it = bids.erase(it);
                } else {
                    break;
                }
            }
        }
    }

    void execute_against_level(Order* incoming, PriceLevel& level, Side resting_side) {
        Order* resting = level.get_head();

        while (resting && incoming->remaining_qty > 0) {
            Quantity exec_qty = std::min(incoming->remaining_qty, resting->remaining_qty);
            Price exec_price = resting->price;

            // Generate trade
            if (on_trade) {
                Trade t;
                t.maker_order_id = resting->id;
                t.taker_order_id = incoming->id;
                t.price = exec_price;
                t.qty = exec_qty;
                t.timestamp_ns = current_time_ns();
                on_trade(t);
            }

            // Update remaining quantities
            incoming->remaining_qty -= exec_qty;
            resting->remaining_qty -= exec_qty;
            level.reduce_volume(exec_qty);

            Order* next_resting = resting->next;

            if (resting->remaining_qty == 0) {
                resting->status = OrderStatus::Filled;
                level.remove(resting);
                order_map.erase(resting->id);
                // Depending on memory lifecycle, return resting to pool
            } else {
                resting->status = OrderStatus::PartiallyFilled;
            }

            if (incoming->remaining_qty == 0) {
                incoming->status = OrderStatus::Filled;
            } else {
                incoming->status = OrderStatus::PartiallyFilled;
            }

            resting = next_resting;
        }
    }
};

} // namespace ullt
