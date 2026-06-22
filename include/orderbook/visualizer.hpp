#pragma once

#include "order_book.hpp"

namespace ob {

class Visualizer {
public:
    void render(const OrderBook& book, int top_n = 10);
};

}  // namespace ob
