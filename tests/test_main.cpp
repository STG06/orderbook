// Self-contained unit tests. No external test framework -- uses <cassert>.
// Build with the rest of the project; run ./build/orderbook_tests.

#include <cassert>
#include <iostream>

#include "orderbook/order_book.hpp"

using namespace ob;

static void test_add_and_best_bid() {
    OrderBook b;
    b.add_order({1, 0, 10000, 5, Side::Bid});
    assert(b.best_bid().value() == 10000);
    assert(!b.best_ask().has_value());
    b.validate();
}

static void test_cancel_clears_book() {
    OrderBook b;
    b.add_order({1, 0, 10000, 5, Side::Bid});
    assert(b.cancel_order(1));
    assert(!b.best_bid().has_value());
    assert(!b.cancel_order(1));
    b.validate();
}

static void test_spread_and_mid() {
    OrderBook b;
    b.add_order({1, 0, 10000, 5, Side::Bid});
    b.add_order({2, 0, 10005, 3, Side::Ask});
    assert(b.spread().value()    == 5);
    assert(b.mid_price().value() == 10002.5);
    b.validate();
}

static void test_fifo_at_same_price() {
    OrderBook b;
    b.add_order({1, 0, 10000, 5, Side::Bid});
    b.add_order({2, 0, 10000, 3, Side::Bid});

    b.execute(1, 4);                              // partial on first
    assert(b.best_bid().value() == 10000);
    b.validate();

    b.execute(1, 1);                              // finish first
    assert(b.best_bid().value() == 10000);        // second still here
    b.validate();

    b.execute(2, 3);                              // finish second
    assert(!b.best_bid().has_value());
    b.validate();
}

static void test_top_of_book_promotes_on_cancel() {
    OrderBook b;
    b.add_order({1, 0, 10000, 5, Side::Bid});
    b.add_order({2, 0,  9999, 3, Side::Bid});
    assert(b.best_bid().value() == 10000);
    b.cancel_order(1);
    assert(b.best_bid().value() == 9999);
    b.validate();
}

static void test_modify_qty_decrease_in_place() {
    OrderBook b;
    b.add_order({1, 0, 10000, 5, Side::Bid});
    b.add_order({2, 0, 10000, 3, Side::Bid});   // queued behind id 1

    assert(b.modify_order(1, 10000, 2));        // shrink id 1, keep priority
    b.execute(1, 2);                            // wipe id 1
    assert(b.best_bid().value() == 10000);      // id 2 still alive
    b.validate();
}

static void test_modify_price_change_repositions() {
    OrderBook b;
    b.add_order({1, 0, 10000, 5, Side::Bid});
    assert(b.modify_order(1, 9998, 5));
    assert(b.best_bid().value() == 9998);
    b.validate();
}

int main() {
    test_add_and_best_bid();
    test_cancel_clears_book();
    test_spread_and_mid();
    test_fifo_at_same_price();
    test_top_of_book_promotes_on_cancel();
    test_modify_qty_decrease_in_place();
    test_modify_price_change_repositions();
    std::cout << "all tests passed\n";
    return 0;
}
