#pragma once

#include <cstdint>

#include "sensigolf/classic_model.hpp"
#include "sensigolf/recovered_flight.hpp"
#include "sensigolf/recovered_prng.hpp"

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
    CupTerminal,
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
    void set_original_interaction_context(
        recovered::OriginalPrng16* prng, bool green_mode,
        std::int16_t flag_x, std::int16_t flag_y,
        bool flag_object_verified = false,
        bool near_cup_distance_zero = false,
        std::uint16_t player_mode_5a = 4) noexcept;
    // Set only after original source mode and distance have been supplied.
    bool original_lip_deflection_last_tick() const noexcept {
        return lip_deflection_last_tick_;
    }
    void update_surface_context_for_tick(
        const ClassicSurfaceContext& context);
    const ClassicSurfaceContext& surface_context() const noexcept;

    void set_ball_origin(std::int32_t x_raw, std::int32_t y_raw) noexcept;

    ClassicShotOutcome outcome() const noexcept {
        return outcome_;
    }

    // Original 0x40A318..0x40A34C: terminate an active flight when the
    // course lookup returns AX=-1 (WORLD boundary). Preserve the ball's
    // actual off-map position for source recovery from the saved safe anchor.
    void stop_for_world_boundary();

    // Synchronize an original game-flow relocation after the shot has ended.
    // This does not calculate physics; it updates the authoritative snapshot
    // to a recovered position supplied by the session layer.
    void relocate_inactive_ball(
        std::int32_t x_raw,
        std::int32_t y_raw,
        std::uint16_t surface_index);

    // Exact v1.014 0x40AB1B / 0x40AB97 ball-coordinate transforms.
    void enter_green_coordinates(
        std::int32_t origin_x_raw,
        std::int32_t origin_y_raw);
    void leave_green_coordinates(
        std::int32_t origin_x_raw,
        std::int32_t origin_y_raw);

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
    recovered::OriginalPrng16* interaction_prng_ = nullptr;
    bool interaction_green_mode_ = false;
    std::int16_t flag_x_ = 0;
    std::int16_t flag_y_ = 0;
    bool flag_object_verified_ = false;
    bool near_cup_distance_zero_ = false;
    std::uint16_t player_mode_5a_ = 4; // original CPU modes >=4 do not lip
    bool lip_deflection_last_tick_ = false;
};

} // namespace sensigolf
