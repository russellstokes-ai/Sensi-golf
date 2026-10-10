#include "sensigolf/recovered_putter_terminal.hpp"

namespace sensigolf::recovered {

bool apply_putter_terminal(
    std::uint16_t landing_code,
    PutterTerminalState& state) noexcept {
    if (landing_code == 8) {
        ++state.counter_52;
        ++state.counter_56;
        state.player_flags =
            static_cast<std::uint16_t>(state.player_flags | 0x0004u);
        state.pause = 0x0064;
        state.event_id = 1;
        state.terminal_flag = 1;
        if (state.player_mode_5a < 4) {
            state.ui_gate = 1;
        }
        state.kind = PutterTerminalKind::Holed;
        return true;
    }

    if (landing_code == 9 || landing_code == 10) {
        state.player_flags =
            static_cast<std::uint16_t>(state.player_flags | 0x0004u);
        state.pause = 0x0064;
        state.event_id = 11;
        state.terminal_flag = 1;
        state.kind = PutterTerminalKind::SpecialGreenStop;
        return true;
    }

    return false;
}

} // namespace sensigolf::recovered
