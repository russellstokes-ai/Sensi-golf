#pragma once

#include <cstdint>

#include "sensigolf/classic_model.hpp"
#include "sensigolf/recovered_flight.hpp"

namespace sensigolf {

struct ClassicSurfaceContext {
    // Original landing/ground event code. Known parity-scoped values include:
    // 0 neutral, 1 green/putter, 2..7 ordinary surfaces, 8 hole,
    // 0x23 immediate-stop hazard.
    std::uint16_t landing_code = 0;
    std::uint16_t slope_direction = 0;
    std::uint16_t slope_magnitude = 0;
};

class ClassicShotModel final : public IClassicModel {
public:
    static constexpr std::uint32_t kTimerInterval16_16 = 0x03A8;
    static constexpr std::uint32_t kNominalTickRateHz = 70;

    void reset() override;
    void begin_shot(const ShotInput& input) override;
    void step() override;
    const BallState& state() const override;
    bool shot_active() const override;
    std::uint32_t tick_rate_hz() const override;

    // Exact recovered scheduler interval used by Windows v1.014.
    std::uint32_t tick_interval_16_16() const noexcept {
        return kTimerInterval16_16;
    }

    void set_surface_context(const ClassicSurfaceContext& context);
    const ClassicSurfaceContext& surface_context() const noexcept;

    // Course/game integration owns the current ball position. Configure it
    // before begin_shot; reset() returns the origin to zero.
    void set_ball_origin(std::int32_t x_raw, std::int32_t y_raw) noexcept;

private:
    void sync_public_state();

    recovered::FlightState flight_{};
    BallState public_{};
    ClassicSurfaceContext surface_{};
    std::int32_t origin_x_ = 0;
    std::int32_t origin_y_ = 0;
    std::uint16_t active_club_ = 0;
    bool active_ = false;
};

} // namespace sensigolf
