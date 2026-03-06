#include <catch2/catch.hpp>
#include "matching_engine.hpp"

using namespace ullt;

TEST_CASE("Matching Engine - Limit Order Matching", "[MatchingEngine]") {
    MatchingEngine me(100);

    // Add Sell order
    OrderId sell1 = me.place_order(10000, 50, Side::Sell, OrderType::Limit);
    REQUIRE(me.get_trades().empty());

    // Add Buy order that crosses
    OrderId buy1 = me.place_order(10050, 50, Side::Buy, OrderType::Limit);

    // Should generate 1 trade
    REQUIRE(me.get_trades().size() == 1);
    
    const Trade& t = me.get_trades()[0];
    REQUIRE(t.maker_order_id == sell1);
    REQUIRE(t.taker_order_id == buy1);
    REQUIRE(t.qty == 50);
    REQUIRE(t.price == 10000); // Maker price
}

TEST_CASE("Matching Engine - Partial Fills", "[MatchingEngine]") {
    MatchingEngine me(100);

    // Sell 100 @ 50
    me.place_order(5000, 100, Side::Sell, OrderType::Limit);

    // Buy 40 @ 50
    me.place_order(5000, 40, Side::Buy, OrderType::Limit);

    REQUIRE(me.get_trades().size() == 1);
    REQUIRE(me.get_trades()[0].qty == 40);
    me.reset_trades();

    // Buy another 70 @ 50 (will fill remaining 60 and rest 10)
    OrderId buy2 = me.place_order(5000, 70, Side::Buy, OrderType::Limit);
    
    REQUIRE(me.get_trades().size() == 1);
    REQUIRE(me.get_trades()[0].qty == 60);
    
    // The Buy order should be resting in the book for 10 qty.
    // Let's sweep it with a market sell.
    me.reset_trades();
    me.place_order(4000, 20, Side::Sell, OrderType::Market); // Assuming Market order supported as aggressive Limit natively or simulated
    
    REQUIRE(me.get_trades().size() == 1);
    REQUIRE(me.get_trades()[0].qty == 10);
    REQUIRE(me.get_trades()[0].maker_order_id == buy2);
}

TEST_CASE("Matching Engine - Order Cancellation", "[MatchingEngine]") {
    MatchingEngine me(100);

    // Add Sell order
    OrderId sell1 = me.place_order(10000, 50, Side::Sell, OrderType::Limit);
    
    // Cancel it
    me.cancel_order(sell1);

    // Attempt to buy against it
    me.place_order(10050, 50, Side::Buy, OrderType::Limit);

    // No trades should occur since the sell was cancelled
    REQUIRE(me.get_trades().empty());
}
