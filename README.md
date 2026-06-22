# orderbook

A limit order book reconstruction engine in C++20. It reads a stream of market
data events (add / cancel / modify / execute), maintains the full live book
state, and exposes the usual top-of-book queries plus a few microstructure
metrics. Ships with a CSV parser, a LOBSTER (Nasdaq) parser, an ANSI terminal
visualizer, and a throughput benchmark.

I built this to brush up on modern C++ and to actually understand what an
exchange matching engine looks like under the hood.

## Build & run

```
cmake -S . -B build
cmake --build build

./build/orderbook_demo data/sample.csv
./build/orderbook_demo data/sample.csv --viz             # live terminal display
./build/orderbook_demo path/to/lobster.csv --lobster
./build/orderbook_tests
```

For the benchmark build Release:

```
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build-rel
cmake --build build-rel
./build-rel/orderbook_bench data/sample.csv
```

## Design

Two `std::map`s of price levels — bids descending (`std::greater<>`), asks
ascending (`std::less<>`) — plus an `unordered_map<OrderID, Locator>` for
O(1) cancel and modify. The locator stores the order's side, its price, and
a `std::list<Order>::iterator` pointing at the order itself.

```
bids_ (sorted high → low)        asks_ (sorted low → high)
  10000 → [o1, o2]                 10005 → [o3]
  9999  → [o4]                     10006 → [o5, o6]
  9998  → [o7]                     10007 → [o8]

index_
  id=1 → {Bid, 10000, list-iter → o1}
  id=2 → {Bid, 10000, list-iter → o2}
  ...
```

Each price level is a `std::list<Order>` in arrival order (time priority).
`std::list` is the right pick here specifically because erasing a node does
not invalidate iterators to other nodes — that's what makes the stored
iterator in the locator safe to use for the lifetime of the order. When a
cancel comes in we look up the locator, jump straight to the list node, and
erase it without scanning the level.

Modify follows the standard convention: a same-price qty *decrease* updates
in place and keeps time priority; anything else (price change, qty increase)
is implemented as cancel + add and loses priority.

`validate()` is a debug-only invariant check that walks both maps and the
index, asserting no empty levels exist, cached `total_qty` matches the sum
of order qtys, and every index entry resolves to a real order. Runs after
every mutation in debug builds; compiles out under `NDEBUG`.

## Layout

```
include/orderbook/   public headers
src/order_book.cpp   add / cancel / modify / execute / best_bid / etc.
src/parser.cpp       CSV and LOBSTER parsers (std::from_chars, no allocation)
src/visualizer.cpp   ANSI top-of-book renderer
src/metrics.cpp      imbalance + rolling VWAP
src/main.cpp         driver: file → parse → dispatch → optional viz
tests/test_main.cpp  unit tests (cassert, no external framework)
benchmarks/          throughput harness
data/sample.csv      tiny hand-written event stream for sanity-checking
```

## Data formats

**CSV (the local format used in `data/sample.csv`):**
```
ts,type,id,side,price,qty
1000,A,1,B,10000,5
1004,X,1,B,10000,2
```
`type`: `A` add, `C` cancel, `M` modify, `X` execute. `side`: `B`/`S`.
Prices are integer ticks.

**LOBSTER message file** (`--lobster`): the standard format from
[lobsterdata.com](https://lobsterdata.com/info/DataStructure.php). Columns
`Time, EventType, OrderID, Size, Price, Direction`. Event-type 7 (trading
halt) is skipped; 4 and 5 (visible / hidden execution) are merged into a
single `Execute` event.

## Notes

- All money uses `int64_t` ticks. No `float`/`double` for prices anywhere
  on the book — only the metrics layer touches doubles.
- Parsing path uses `std::string_view` + `std::from_chars`, no heap
  allocation per row.
- The visualizer uses raw ANSI escape codes. Works in Windows Terminal,
  iTerm2, and VS Code's integrated terminal out of the box. Legacy
  `cmd.exe` would need `SetConsoleMode(..., ENABLE_VIRTUAL_TERMINAL_PROCESSING)`.

## Features

- Full add / cancel / modify / execute event handling with debug-time
  invariant checking
- CSV and LOBSTER (Nasdaq) parsers, zero allocation per row
- ANSI terminal visualizer with top-of-book depth bars
- Microstructure metrics: spread, mid, top-of-book imbalance, rolling VWAP
- Self-contained unit tests (no external test framework dependency)
- Release-mode throughput benchmark with warm-up pass and checksum
  anti-dead-code-elimination guard
