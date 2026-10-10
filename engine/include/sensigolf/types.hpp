#pragma once

#include <cstdint>

namespace sensigolf {

// Units are deliberately "raw" until the original engine representation is
// recovered. Do not assume pixels, yards, metres or 16.16 fixed point.
using RawScalar = std::int64_t;
using Tick = std::uint32_t;

enum class ShotPhase : std::uint8_t {
    Idle = 0,
    Airborne,
    GroundRoll,
    Complete,
};

struct ShotInput {
    RawScalar aim_raw = 0;
    std::int32_t power_tick = 0;
    std::int32_t accuracy_tick = 0;
    std::uint16_t club_index = 0;
    std::uint16_t lie_index = 0;
};

struct BallState {
    Tick tick = 0;

    RawScalar x_raw = 0;
    RawScalar y_raw = 0;
    RawScalar z_raw = 0;

    RawScalar vx_raw = 0;
    RawScalar vy_raw = 0;
    RawScalar vz_raw = 0;

    std::uint16_t surface_index = 0;
    ShotPhase phase = ShotPhase::Idle;

    bool hazard = false;
    bool holed = false;
};

inline bool operator==(const BallState& a, const BallState& b) {
    return a.tick == b.tick
        && a.x_raw == b.x_raw
        && a.y_raw == b.y_raw
        && a.z_raw == b.z_raw
        && a.vx_raw == b.vx_raw
        && a.vy_raw == b.vy_raw
        && a.vz_raw == b.vz_raw
        && a.surface_index == b.surface_index
        && a.phase == b.phase
        && a.hazard == b.hazard
        && a.holed == b.holed;
}

inline bool operator!=(const BallState& a, const BallState& b) {
    return !(a == b);
}

} // namespace sensigolf
