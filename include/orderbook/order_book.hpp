#pragma once

#include <functional>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>

#include "price_level.hpp"
#include "types.hpp"

namespace ob {

class OrderBook {
public:
    using BidMap = std::map<Price, PriceLevel, std::greater<>>;
    using AskMap = std::map<Price, PriceLevel, std::less<>>;

    void add_order(const Order& o);
    bool cancel_order(OrderID id);
    bool modify_order(OrderID id, Price new_price, Qty new_qty);
    void execute(OrderID id, Qty filled);

    std::optional<Price>  best_bid()  const;
    std::optional<Price>  best_ask()  const;
    std::optional<Price>  spread()    const;
    std::optional<double> mid_price() const;

    const BidMap& bids() const noexcept { return bids_; }
    const AskMap& asks() const noexcept { return asks_; }

    void validate() const;

private:
    struct OrderLocator {
        Side  side;
        Price price;
        std::list<Order>::iterator it;
    };

    BidMap bids_;
    AskMap asks_;
    std::unordered_map<OrderID, OrderLocator> index_;
};

}  // namespace ob
