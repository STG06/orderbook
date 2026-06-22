#pragma once

#include <deque>
#include <optional>
#include <utility>

#include "order_book.hpp"

namespace ob {

// (top_bid_qty - top_ask_qty) / (top_bid_qty + top_ask_qty), in [-1, +1].
std::optional<double> imbalance(const OrderBook& book);

// Rolling volume-weighted average price over the last N executed trades.
class VWAP {
public:
    explicit VWAP(std::size_t window) : window_(window) {}

    void on_trade(Price p, Qty q);
    std::optional<double> value() const;

private:
    std::size_t window_;
    std::deque<std::pair<Price, Qty>> trades_;
    double px_qty_sum_ = 0.0;
    double qty_sum_    = 0.0;
};

}  // namespace ob
