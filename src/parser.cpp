#include "orderbook/parser.hpp"

#include <charconv>

namespace ob {

static std::string_view next_token(std::string_view& line) {
    auto comma = line.find(',');
    auto tok = line.substr(0, comma);
    line.remove_prefix(comma == std::string_view::npos ? line.size() : comma + 1);
    return tok;
}

template <typename T>
static bool parse_int(std::string_view s, T& out) {
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), out);
    return ec == std::errc{};
}

std::optional<MarketEvent> parse_csv_row(std::string_view line) {
    MarketEvent ev{};

    auto ts_tok = next_token(line);
    if (!parse_int(ts_tok, ev.ts)) return std::nullopt;

    auto type_tok = next_token(line);
    if      (type_tok == "A") ev.type = EventType::Add;
    else if (type_tok == "C") ev.type = EventType::Cancel;
    else if (type_tok == "M") ev.type = EventType::Modify;
    else if (type_tok == "X") ev.type = EventType::Execute;
    else return std::nullopt;

    auto id_tok = next_token(line);
    if (!parse_int(id_tok, ev.id)) return std::nullopt;

    auto side_tok = next_token(line);
    if      (side_tok == "B") ev.side = Side::Bid;
    else if (side_tok == "S") ev.side = Side::Ask;
    else return std::nullopt;

    auto price_tok = next_token(line);
    if (!parse_int(price_tok, ev.price)) return std::nullopt;

    auto qty_tok = next_token(line);
    if (!parse_int(qty_tok, ev.qty)) return std::nullopt;

    return ev;
}

std::optional<MarketEvent> parse_lobster_row(std::string_view line) {
    MarketEvent ev{};

    auto ts_tok = next_token(line);
    if (!parse_int(ts_tok, ev.ts)) return std::nullopt;

    int etype = 0;
    auto etype_tok = next_token(line);
    if (!parse_int(etype_tok, etype)) return std::nullopt;
    if      (etype == 1)               ev.type = EventType::Add;
    else if (etype == 2)               ev.type = EventType::Modify;
    else if (etype == 3)               ev.type = EventType::Cancel;
    else if (etype == 4 || etype == 5) ev.type = EventType::Execute;
    else return std::nullopt;   // halt or unknown -- skip

    auto id_tok = next_token(line);
    if (!parse_int(id_tok, ev.id)) return std::nullopt;

    auto qty_tok = next_token(line);
    if (!parse_int(qty_tok, ev.qty)) return std::nullopt;

    auto price_tok = next_token(line);
    if (!parse_int(price_tok, ev.price)) return std::nullopt;

    int dir = 0;
    auto dir_tok = next_token(line);
    if (!parse_int(dir_tok, dir)) return std::nullopt;
    if      (dir ==  1) ev.side = Side::Bid;
    else if (dir == -1) ev.side = Side::Ask;
    else return std::nullopt;

    return ev;
}

}  // namespace ob
