#include <fstream>
#include <iostream>
#include <string>

#include "orderbook/metrics.hpp"
#include "orderbook/order_book.hpp"
#include "orderbook/parser.hpp"
#include "orderbook/visualizer.hpp"

using namespace ob;

static void apply(OrderBook& book, VWAP& vwap, const MarketEvent& e) {
    switch (e.type) {
        case EventType::Add:
            book.add_order({e.id, e.ts, e.price, e.qty, e.side});
            break;
        case EventType::Cancel:
            book.cancel_order(e.id);
            break;
        case EventType::Execute:
            book.execute(e.id, e.qty);
            vwap.on_trade(e.price, e.qty);
            break;
        case EventType::Modify:
            book.modify_order(e.id, e.price, e.qty);
            break;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <data-file> [--lobster] [--viz]\n";
        return 1;
    }

    bool use_lobster = false;
    bool show_viz    = false;
    for (int i = 2; i < argc; ++i) {
        std::string a = argv[i];
        if      (a == "--lobster") use_lobster = true;
        else if (a == "--viz")     show_viz    = true;
    }

    std::ifstream in(argv[1]);
    if (!in) {
        std::cerr << "could not open " << argv[1] << "\n";
        return 1;
    }

    OrderBook  book;
    Visualizer viz;
    VWAP       vwap(100);

    std::string line;
    if (!use_lobster) std::getline(in, line);   // skip CSV header row

    std::size_t parsed = 0, applied = 0;
    while (std::getline(in, line)) {
        auto ev = use_lobster ? parse_lobster_row(line) : parse_csv_row(line);
        if (!ev) continue;
        ++parsed;

        apply(book, vwap, *ev);
        ++applied;

        if (show_viz && (applied % 100 == 0)) viz.render(book, 10);
    }

    if (show_viz) viz.render(book, 10);

    std::cout << "\nparsed " << parsed << " events, applied " << applied << "\n";
    if (auto p = book.best_bid())  std::cout << "best bid:  " << *p << "\n";
    if (auto p = book.best_ask())  std::cout << "best ask:  " << *p << "\n";
    if (auto p = book.spread())    std::cout << "spread:    " << *p << "\n";
    if (auto p = book.mid_price()) std::cout << "mid:       " << *p << "\n";
    if (auto p = imbalance(book))  std::cout << "imbalance: " << *p << "\n";
    if (auto p = vwap.value())     std::cout << "vwap(100): " << *p << "\n";

    return 0;
}
