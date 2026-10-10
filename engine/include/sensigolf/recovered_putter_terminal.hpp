#pragma once

#include <cstdint>

namespace sensigolf::recovered {

enum class PutterTerminalKind : std::uint8_t {
    None = 0,
    Holed,
    SpecialGreenStop,
};

struct PutterTerminalState {
    std::uint16_t player_flags = 0;
    std::uint16_t counter_52 = 0;
    std::uint16_t counter_56 = 0;
    std::uint16_t player_mode_5a = 0;
    std::uint16_t pause = 0;
    std::uint16_t terminal_flag = 0;
    std::uint8_t ui_gate = 0;
    std::uint16_t event_id = 0xFFFF;
    PutterTerminalKind kind = PutterTerminalKind::None;
};

// Recovered v1.014 club-12 terminal branches reached after terrain lookup.
// Returns true only for landing codes 8, 9 and 10.
bool apply_putter_terminal(
    std::uint16_t landing_code,
    PutterTerminalState& state) noexcept;

} // namespace sensigolf::recovered
