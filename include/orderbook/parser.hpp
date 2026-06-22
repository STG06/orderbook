#pragma once

#include <optional>
#include <string_view>

#include "types.hpp"

namespace ob {

enum class EventType : uint8_t {
    Add,
    Cancel,
    Modify,
    Execute,
};

struct MarketEvent {
    Timestamp ts;
    OrderID   id;
    EventType type;
    Side      side;
    Price     price;
    Qty       qty;
};

// CSV format:  ts,type,id,side,price,qty
// type codes:  A=Add  C=Cancel  M=Modify  X=Execute
// side codes:  B=Bid  S=Ask
std::optional<MarketEvent> parse_csv_row(std::string_view line);

// LOBSTER message file format:  Time,EventType,OrderID,Size,Price,Direction
// EventType: 1=Add 2=Modify 3=Cancel 4/5=Execute 7=halt (skipped)
// Direction: +1=Bid -1=Ask
std::optional<MarketEvent> parse_lobster_row(std::string_view line);

}  // namespace ob
