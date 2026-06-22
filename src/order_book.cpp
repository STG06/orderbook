#include "orderbook/order_book.hpp"

#include <cassert>

namespace ob {

void OrderBook::add_order(const Order& o) {
    auto [lvl_it, inserted] = (o.side == Side::Bid)
        ? bids_.try_emplace(o.price)
        : asks_.try_emplace(o.price);

    auto order_it = lvl_it->second.add(o);
    index_.emplace(o.id, OrderLocator{o.side, o.price, order_it});
}

bool OrderBook::cancel_order(OrderID id) {
    auto it = index_.find(id);
    if (it == index_.end()) return false;

    auto& loc = it->second;

    if (loc.side == Side::Bid) {
        auto lvl_it = bids_.find(loc.price);
        lvl_it->second.remove(loc.it);
        if (lvl_it->second.empty()) bids_.erase(lvl_it);
    } else {
        auto lvl_it = asks_.find(loc.price);
        lvl_it->second.remove(loc.it);
        if (lvl_it->second.empty()) asks_.erase(lvl_it);
    }

    index_.erase(it);
    return true;
}

void OrderBook::execute(OrderID id, Qty filled) {
    auto it = index_.find(id);
    if (it == index_.end()) return;

    auto& loc = it->second;
    auto lvl_it = (loc.side == Side::Bid) ? bids_.find(loc.price)
                                          : asks_.find(loc.price);
    auto& level = lvl_it->second;

    // clamp to remaining qty so total_qty stays consistent even on over-fills
    Qty actual = (filled > loc.it->qty) ? loc.it->qty : filled;
    loc.it->qty   -= actual;
    level.total_qty -= actual;

    if (loc.it->qty <= 0) {
        // erase directly -- total_qty is already up to date
        level.orders.erase(loc.it);
        if (level.empty()) {
            if (loc.side == Side::Bid) bids_.erase(lvl_it);
            else                       asks_.erase(lvl_it);
        }
        index_.erase(it);
    }
}

bool OrderBook::modify_order(OrderID id, Price new_price, Qty new_qty) {
    auto it = index_.find(id);
    if (it == index_.end()) return false;

    auto& loc = it->second;
    Qty old_qty = loc.it->qty;

    // qty-decrease at the same price keeps time priority
    if (new_price == loc.price && new_qty < old_qty) {
        Qty diff = old_qty - new_qty;
        loc.it->qty = new_qty;
        if (loc.side == Side::Bid) bids_.find(loc.price)->second.total_qty -= diff;
        else                       asks_.find(loc.price)->second.total_qty -= diff;
        return true;
    }

    // otherwise: cancel + add (lose time priority)
    Order replacement = *loc.it;
    replacement.price = new_price;
    replacement.qty   = new_qty;
    cancel_order(id);
    add_order(replacement);
    return true;
}

std::optional<Price> OrderBook::best_bid() const {
    if (bids_.empty()) return std::nullopt;
    return bids_.begin()->first;
}

std::optional<Price> OrderBook::best_ask() const {
    if (asks_.empty()) return std::nullopt;
    return asks_.begin()->first;
}

std::optional<Price> OrderBook::spread() const {
    auto b = best_bid();
    auto a = best_ask();
    if (!b || !a) return std::nullopt;
    return *a - *b;
}

std::optional<double> OrderBook::mid_price() const {
    auto b = best_bid();
    auto a = best_ask();
    if (!b || !a) return std::nullopt;
    return (*b + *a) / 2.0;
}

void OrderBook::validate() const {
#ifndef NDEBUG
    for (auto& [price, level] : bids_) {
        assert(!level.empty());
        Qty sum = 0;
        for (auto& o : level.orders) sum += o.qty;
        assert(sum == level.total_qty);
    }
    for (auto& [price, level] : asks_) {
        assert(!level.empty());
        Qty sum = 0;
        for (auto& o : level.orders) sum += o.qty;
        assert(sum == level.total_qty);
    }
    for (auto& [id, loc] : index_) {
        if (loc.side == Side::Bid) {
            auto lvl_it = bids_.find(loc.price);
            assert(lvl_it != bids_.end());
        } else {
            auto lvl_it = asks_.find(loc.price);
            assert(lvl_it != asks_.end());
        }
        assert(loc.it->id == id);
    }
#endif
}

}  // namespace ob
