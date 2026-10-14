#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "sensigolf/classic_course_resources.hpp"
#include "sensigolf/classic_game_session.hpp"
#include "sensigolf/classic_mobile_controls.hpp"
#include "sensigolf/classic_replay_journal.hpp"
#include "sensigolf/recovered_prng.hpp"
#include "recovered_welly_tick.hpp"
#include "pixtee_touch_meter.hpp"

namespace pixtee {
// Presentation-only sampled SOURCE MAPS slope, never fabricated by art code.
struct NativeGreenArrowCell {
    std::int16_t world_x = 0;
    std::int16_t world_y = 0;
    std::uint16_t slope_direction_raw = 0; // 0..4095, 16 sector steps of 256
    std::uint16_t slope_magnitude = 0;     // original 0..15
    std::uint16_t landing_code = 0;        // GREEN H4 is 1
};
// Headless native host ONLY: never calculates flight, collision, scoring, RNG,
// hazard, round progression or physics. Delegates all of it to recovered C++.
// Caller supplies independently authored original-FORMAT course data;
// no original commercial course files are bundled here.
class PixteeRoundRuntime final {
public:
    PixteeRoundRuntime(
        std::array<std::uint8_t,18> hole_order,
        std::array<std::uint8_t,100> pars,
        std::uint16_t seed0, std::uint16_t seed1);
    void register_course(std::uint8_t resource_id,
                         const sensigolf::ClassicCourseResources& course);
    bool course_registered(std::uint8_t resource_id) const noexcept;
    void load_next_hole();
    void set_aim_raw(std::uint16_t raw);
    void nudge_aim_raw(std::int32_t delta);
    void set_club(std::uint16_t index);
    void set_verified_flag_object(bool present);
    void set_soft_ground_enabled(bool enabled);
    void set_authored_birds_enabled(bool enabled);
    void set_authored_tree_objects(const std::vector<sensigolf::AuthoredTreeVolume>& trees);
    void set_original_player_mode_5a(std::uint16_t source_mode);
    void cycle_club(int delta);
    void meter_click_raw(std::uint16_t sample);
    void start_touch_meter();
    bool tick_touch_meter(bool pressed);
    const TouchMeter& touch_meter() const noexcept {return touch_meter_;}
    void start_original_welly();
    bool tick_original_welly(bool pressed);
    bool original_welly_active() const noexcept;
    bool original_welly_rejected() const noexcept;
    const OriginalWellyTickState& original_welly_state() const noexcept;
    void cancel_meter() noexcept;
    void step_tick();
    void acknowledge_hazard();
    void acknowledge_special_green();
    void commit_scored_hole();
    std::string serialize_stable_replay() const;
    std::string serialize_native_session() const;
    void restore_native_session(const std::string& checkpoint);
    std::string native_fingerprint() const;
    void restore_replay(const std::string& checkpoint);
    const sensigolf::ClassicRoundState& round_state() const noexcept;
    std::optional<sensigolf::ClassicHolePlanEntry> resource_request() const;
    bool has_hole() const noexcept;
    const sensigolf::ClassicHoleSession& hole() const;
    std::optional<sensigolf::ClassicGreenRegion> active_green_region() const;
    std::vector<NativeGreenArrowCell> verified_native_green_arrows(
        std::uint16_t green_detail_stride = 8) const;
    sensigolf::ClassicMeterStage meter_stage() const noexcept;
    bool controls_enabled() const noexcept;
    std::uint16_t aim_raw() const noexcept;
    std::uint16_t club_index() const noexcept;
    std::size_t saved_shots() const noexcept;
    bool replay_supported() const noexcept;

private:
    struct Command {char op;std::vector<std::int64_t> args;std::uint32_t repeat=1;};
    void record_command(char op,std::vector<std::int64_t> args={});
    std::vector<Command> commands_;
    std::array<std::uint8_t,100> pars_{};
    bool recording_=true;
    void record_charged_no_flight(const sensigolf::ClassicShotRequest& request);
    void maybe_record_stable_shot();
    sensigolf::ClassicHolePlan plan_;
    sensigolf::recovered::OriginalPrng16 initial_seed_;
    std::unique_ptr<sensigolf::ClassicGameSession> game_;
    std::map<std::uint8_t,sensigolf::ClassicCourseResources> courses_;
    sensigolf::ClassicMobileControls controls_;
    TouchMeter touch_meter_{};
    OriginalWellyTick original_welly_{};
    bool original_welly_active_=false;
    bool original_welly_first_press_=false;
    bool original_welly_rejected_=false;
    sensigolf::ClassicReplayJournal journal_;
    std::optional<sensigolf::ClassicShotRequest> pending_;
    bool replay_supported_ = true;
};
} // namespace pixtee
