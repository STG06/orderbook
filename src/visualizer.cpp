#include "orderbook/visualizer.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace ob {

namespace {
constexpr int BAR_WIDTH = 40;

std::string bar(Qty q, Qty max_q) {
    if (max_q <= 0) return {};
    int w = static_cast<int>((q * BAR_WIDTH) / max_q);
    return std::string(w, '#');
}
}  // namespace

void Visualizer::render(const OrderBook& book, int top_n) {
    std::vector<std::pair<Price, Qty>> top_asks;
    std::vector<std::pair<Price, Qty>> top_bids;

    int n = 0;
    for (auto const& [p, lvl] : book.asks()) {
        if (n++ >= top_n) break;
        top_asks.push_back({p, lvl.total_qty});
    }
    n = 0;
    for (auto const& [p, lvl] : book.bids()) {
        if (n++ >= top_n) break;
        top_bids.push_back({p, lvl.total_qty});
    }

    Qty max_q = 1;
    for (auto& [p, q] : top_asks) max_q = std::max(max_q, q);
    for (auto& [p, q] : top_bids) max_q = std::max(max_q, q);

    std::cout << "\033[2J\033[H";

    for (auto it = top_asks.rbegin(); it != top_asks.rend(); ++it) {
        std::cout << "\033[31mASK " << it->first
                  << "  " << bar(it->second, max_q)
                  << " " << it->second << "\033[0m\n";
    }

    std::cout << "------------------------------------------------------------\n";
    auto bid = book.best_bid();
    auto ask = book.best_ask();
    auto spr = book.spread();
    auto mid = book.mid_price();
    if (bid && ask) {
        std::cout << "bid=" << *bid
                  << "  ask=" << *ask
                  << "  spread=" << *spr
                  << "  mid=" << *mid << "\n";
    } else {
        std::cout << "(one side empty)\n";
    }
    std::cout << "------------------------------------------------------------\n";

    for (auto& [p, q] : top_bids) {
        std::cout << "\033[32mBID " << p
                  << "  " << bar(q, max_q)
                  << " " << q << "\033[0m\n";
    }

    std::cout.flush();
}

}  // namespace ob
