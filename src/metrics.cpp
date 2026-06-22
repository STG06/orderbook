#include "orderbook/metrics.hpp"

namespace ob {

std::optional<double> imbalance(const OrderBook& book) {
    if (book.bids().empty() || book.asks().empty()) return std::nullopt;
    Qty b = book.bids().begin()->second.total_qty;
    Qty a = book.asks().begin()->second.total_qty;
    if (b + a == 0) return std::nullopt;
    return static_cast<double>(b - a) / static_cast<double>(b + a);
}

void VWAP::on_trade(Price p, Qty q) {
    trades_.push_back({p, q});
    px_qty_sum_ += static_cast<double>(p) * static_cast<double>(q);
    qty_sum_    += static_cast<double>(q);

    while (trades_.size() > window_) {
        auto [op, oq] = trades_.front();
        px_qty_sum_ -= static_cast<double>(op) * static_cast<double>(oq);
        qty_sum_    -= static_cast<double>(oq);
        trades_.pop_front();
    }
}

std::optional<double> VWAP::value() const {
    if (qty_sum_ <= 0.0) return std::nullopt;
    return px_qty_sum_ / qty_sum_;
}

}  // namespace ob
