#pragma once

#include <iterator>
#include <list>

#include "types.hpp"

namespace ob {

struct PriceLevel {
    std::list<Order> orders;
    Qty total_qty = 0;

    std::list<Order>::iterator add(const Order& o) {
        total_qty += o.qty;
        orders.push_back(o);
        return std::prev(orders.end());
    }

    void remove(std::list<Order>::iterator it) {
        total_qty -= it->qty;
        orders.erase(it);
    }

    bool empty() const noexcept { return orders.empty(); }
};

}  // namespace ob
