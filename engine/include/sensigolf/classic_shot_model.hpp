#pragma once

#include <cstdint>

#include "sensigolf/classic_model.hpp"
#include "sensigolf/recovered_flight.hpp"

namespace sensigolf {

struct ClassicSurfaceContext {
    std::uint16_t landing_code = 0;
    std::uint16_t slope_direction = 0;
    std::uint16_t slope_magnitude = 0;
};

enum class ClassicShotOutcome : std::uint8_t {
    None = 0,
    Rest,
    Hazard,
    Holed,
    SpecialGreenStop,
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

    std::uint32_t tick_interval_16_16() const noexcept {
        return kTimerInterval16_16;
    }

    void set_surface_context(const ClassicSurfaceContext& context);
    void update_surface_context_for_tick(
        const ClassicSurfaceContext& context);
    const ClassicSurfaceContext& surface_context() const noexcept;

    void set_ball_origin(std::int32_t x_raw, std::int32_t y_raw) noexcept;

    ClassicShotOutcome outcome() const noexcept {
        return outcome_;
    }

private:
    void sync_public_state();

    recovered::FlightState flight_{};
    BallState public_{};
    ClassicSurfaceContext surface_{};
    std::int32_t origin_x_ = 0;
    std::int32_t origin_y_ = 0;
    std::uint16_t active_club_ = 0;
    ClassicShotOutcome outcome_ = ClassicShotOutcome::None;
    bool active_ = false;
};

} // namespace sensigolf
