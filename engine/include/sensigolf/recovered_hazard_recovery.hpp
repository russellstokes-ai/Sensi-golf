#pragma once

#include <cstdint>

namespace sensigolf::recovered {

struct HazardRecoveryInput {
    std::int32_t ball_x = 0;
    std::int32_t ball_y = 0;
    std::int32_t safe_anchor_x = 0;
    std::int32_t safe_anchor_y = 0;
    std::int32_t x_extent = 0;
    std::int32_t y_extent = 0;
    bool green_mode = false;
};

struct HazardRecoveryState {
    std::int32_t ball_x = 0;
    std::int32_t ball_y = 0;
    std::int32_t player_x = 0;
    std::int32_t player_y = 0;
    std::int32_t player_last_x = 0;
    std::int32_t player_last_y = 0;
    std::int32_t height = 0;
    std::uint16_t pause = 0;
    std::uint16_t player_flags = 0;
    bool used_safe_anchor = false;
};

// Original v1.014 branch 0x40A7E0..0x40A8D6 after the hazard pause expires.
// This models only authoritative position/state recovery. Penalty/scoring
// semantics are deliberately outside this primitive until separately proven.
HazardRecoveryState recover_hazard_position(
    const HazardRecoveryInput& input) noexcept;

} // namespace sensigolf::recovered
