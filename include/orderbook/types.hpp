#pragma once

#include <cstdint>

namespace ob {

using Price     = int64_t;
using Qty       = int64_t;
using OrderID   = uint64_t;
using Timestamp = uint64_t;

enum class Side : uint8_t { Bid, Ask };

struct Order {
    OrderID   id;
    Timestamp ts;
    Price     price;
    Qty       qty;
    Side      side;
};

}  // namespace ob
