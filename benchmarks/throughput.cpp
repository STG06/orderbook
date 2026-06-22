#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "orderbook/order_book.hpp"
#include "orderbook/parser.hpp"

using namespace ob;

static void replay(OrderBook& b, const std::vector<MarketEvent>& events,
                   uint64_t& checksum) {
    for (auto const& e : events) {
        switch (e.type) {
            case EventType::Add:
                b.add_order({e.id, e.ts, e.price, e.qty, e.side});
                break;
            case EventType::Cancel:
                b.cancel_order(e.id);
                break;
            case EventType::Execute:
                b.execute(e.id, e.qty);
                break;
            case EventType::Modify:
                b.modify_order(e.id, e.price, e.qty);
                break;
        }
        if (auto p = b.best_bid()) checksum ^= static_cast<uint64_t>(*p);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <data-file> [--lobster]\n";
        return 1;
    }
    bool use_lobster = (argc > 2 && std::string(argv[2]) == "--lobster");

    std::vector<MarketEvent> events;
    events.reserve(2'000'000);
    {
        std::ifstream in(argv[1]);
        if (!in) { std::cerr << "could not open " << argv[1] << "\n"; return 1; }
        std::string line;
        if (!use_lobster) std::getline(in, line);
        while (std::getline(in, line)) {
            auto e = use_lobster ? parse_lobster_row(line) : parse_csv_row(line);
            if (e) events.push_back(*e);
        }
    }
    std::cout << "loaded " << events.size() << " events\n";

    // warm-up pass (untimed) so the timed run hits a hot allocator / cache
    {
        OrderBook b;
        uint64_t throwaway = 0;
        replay(b, events, throwaway);
    }

    using clock = std::chrono::steady_clock;
    auto t0 = clock::now();

    OrderBook book;
    uint64_t checksum = 0;
    replay(book, events, checksum);

    auto t1 = clock::now();
    double secs = std::chrono::duration<double>(t1 - t0).count();
    double mps  = static_cast<double>(events.size()) / secs / 1e6;

    std::cout << events.size() << " events in " << secs << " s = "
              << mps << " M msg/s  (cksum=" << checksum << ")\n";
    return 0;
}
