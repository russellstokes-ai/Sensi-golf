#pragma once

#include "sensigolf/types.hpp"

namespace sensigolf {

// Contract for the recovered classic simulation.
//
// There is intentionally no default/approximate implementation. A concrete
// implementation is added only when original behaviour has been recovered.
class IClassicModel {
public:
    virtual ~IClassicModel() = default;

    // Reset all shot/gameplay simulation state to a known deterministic state.
    virtual void reset() = 0;

    // Initialize one shot from recovered original-game inputs.
    virtual void begin_shot(const ShotInput& input) = 0;

    // Advance exactly one original logical simulation tick.
    virtual void step() = 0;

    // Read the authoritative current simulation state.
    virtual const BallState& state() const = 0;

    // True while the shot still requires simulation ticks.
    virtual bool shot_active() const = 0;

    // Original logical update rate once recovered. Zero means unknown and is
    // not valid for a production implementation.
    virtual std::uint32_t tick_rate_hz() const = 0;
};

} // namespace sensigolf
