#pragma once
#include "sensigolf/authored_wind.hpp"
#include "sensigolf/authored_tree_contacts.hpp"
#include "sensigolf/authored_bird.hpp"

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

// Read-only presentation evidence captured at the actual recovered branch.
// It never participates in physics, PRNG, scoring or collision decisions.
struct ClassicPresentationContact {
    std::uint32_t bird_id=0,bird_member=0;
    std::uint32_t tree_id=0;
    bool canopy=false;
    bool impact = false;
    bool flag = false;
    bool lip = false;
    bool hazard = false;
    bool green_mode = false;
    std::uint16_t landing_code = 0;
    std::int32_t x_raw = 0, y_raw = 0;
};

class ClassicShotModel final : public IClassicModel {
public:
    void set_authored_wind(const AuthoredWindState& value) {validate_authored_wind(value);wind_=value;wind_accumulator_={};}
    const AuthoredWindState& authored_wind() const noexcept {return wind_;}
    const AuthoredWindAccumulator& wind_accumulator() const noexcept {return wind_accumulator_;}
    void set_soft_ground_enabled(bool enabled) noexcept {soft_ground_enabled_=enabled;}
    void set_authored_bird_context(AuthoredBirdSample before,AuthoredBirdSample after,bool* hit) noexcept {
        bird_before_=before;bird_after_=after;bird_hit_=hit;
    }
    void set_authored_tree_context(const std::vector<AuthoredTreeVolume>* trees,
        std::int32_t origin_x,std::int32_t origin_y,bool green) noexcept {
        authored_trees_=trees;tree_origin_x_=origin_x;tree_origin_y_=origin_y;tree_green_=green;
    }
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
    const ClassicPresentationContact& presentation_contact() const noexcept {
        return presentation_contact_;
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
    AuthoredWindState wind_{};
    AuthoredWindAccumulator wind_accumulator_{};
    AuthoredBirdSample bird_before_{},bird_after_{};
    bool* bird_hit_=nullptr;
    void sync_public_state();

    recovered::FlightState flight_{};
    const std::vector<AuthoredTreeVolume>* authored_trees_=nullptr;
    std::int32_t tree_origin_x_=0,tree_origin_y_=0;
    bool tree_green_=false;
    std::uint32_t last_tree_id_=0;
    BallState public_{};
    ClassicSurfaceContext surface_{};
    std::int32_t origin_x_ = 0;
    std::int32_t origin_y_ = 0;
    std::uint16_t active_club_ = 0;
    ClassicShotOutcome outcome_ = ClassicShotOutcome::None;
    bool active_ = false;
    bool soft_ground_enabled_=false;
    bool soft_ground_rolling_=false;
    recovered::OriginalPrng16* interaction_prng_ = nullptr;
    bool interaction_green_mode_ = false;
    std::int16_t flag_x_ = 0;
    std::int16_t flag_y_ = 0;
    bool flag_object_verified_ = false;
    bool near_cup_distance_zero_ = false;
    std::uint16_t player_mode_5a_ = 4; // original CPU modes >=4 do not lip
    bool lip_deflection_last_tick_ = false;
    ClassicPresentationContact presentation_contact_{};
};

} // namespace sensigolf
