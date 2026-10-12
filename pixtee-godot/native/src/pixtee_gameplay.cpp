#include "pixtee_gameplay.hpp"
#include "pixtee_course_adapter.hpp"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include "godot_cpp/core/class_db.hpp"

namespace godot {
void PixteeGameplay::_bind_methods() {
    ClassDB::bind_method(D_METHOD("load_lakewood_hole1"), &PixteeGameplay::load_lakewood_hole1);
    ClassDB::bind_method(D_METHOD("restart_hole"), &PixteeGameplay::restart_hole);
    ClassDB::bind_method(D_METHOD("set_aim_raw", "heading"), &PixteeGameplay::set_aim_raw);
    ClassDB::bind_method(D_METHOD("nudge_aim_raw", "delta"), &PixteeGameplay::nudge_aim_raw);
    ClassDB::bind_method(D_METHOD("set_club_index", "index"), &PixteeGameplay::set_club_index);
    ClassDB::bind_method(D_METHOD("cycle_club", "delta"), &PixteeGameplay::cycle_club);
    ClassDB::bind_method(D_METHOD("meter_click_raw", "sample"), &PixteeGameplay::meter_click_raw);
    ClassDB::bind_method(D_METHOD("cancel_meter"), &PixteeGameplay::cancel_meter);
    ClassDB::bind_method(D_METHOD("acknowledge_hazard_recovery"), &PixteeGameplay::acknowledge_hazard_recovery);
    ClassDB::bind_method(D_METHOD("acknowledge_special_green_stop"), &PixteeGameplay::acknowledge_special_green_stop);
    ClassDB::bind_method(D_METHOD("step_tick"), &PixteeGameplay::step_tick);
    ClassDB::bind_method(D_METHOD("step_ticks", "count"), &PixteeGameplay::step_ticks);
    ClassDB::bind_method(D_METHOD("snapshot"), &PixteeGameplay::snapshot);
    ClassDB::bind_method(D_METHOD("rules_contract"), &PixteeGameplay::rules_contract);
    ClassDB::bind_method(D_METHOD("last_error"), &PixteeGameplay::last_error);
}
bool PixteeGameplay::load_lakewood_hole1() {
    hole_.reset();
    course_.reset();
    controls_.reset();
    last_error_ = "";
    try {
        course_ = std::make_unique<sensigolf::ClassicCourseResources>(
            pixtee::make_lakewood_hole_1());
        hole_ = std::make_unique<sensigolf::ClassicHoleSession>(
            *course_, std::size_t{0}, sensigolf::ClassicHoleMetadata{0,4,0});
        controls_.sync_hole(*hole_);
        return true;
    } catch (const std::exception& e) {
        last_error_ = String(e.what());
        hole_.reset();
        course_.reset();
        return false;
    }
}
bool PixteeGameplay::restart_hole() {
    if (!hole_ || !course_) return false;
    try {
        const auto tee = course_->player_start(0);
        hole_->reset(tee.x_raw(), tee.y_raw());
        controls_.reset();
        controls_.sync_hole(*hole_);
        last_error_ = "";
        return true;
    } catch (const std::exception& e) {
        last_error_ = String(e.what());
        return false;
    }
}
bool PixteeGameplay::set_aim_raw(int raw) {
    if (!hole_ || raw < 0 || raw > 4095) return false;
    try {
        controls_.set_aim_raw(static_cast<std::uint16_t>(raw));
        last_error_ = "";
        return true;
    } catch (const std::exception& e) {
        last_error_ = String(e.what());
        return false;
    }
}
bool PixteeGameplay::nudge_aim_raw(int delta) {
    if (!hole_) return false;
    try {
        controls_.nudge_aim(delta);
        last_error_ = "";
        return true;
    } catch (const std::exception& e) {
        last_error_ = String(e.what());
        return false;
    }
}
bool PixteeGameplay::set_club_index(int index) {
    if (!hole_ || index < 0 || index > 12) return false;
    try {
        controls_.set_club(static_cast<std::uint16_t>(index));
        last_error_ = "";
        return true;
    } catch (const std::exception& e) {
        last_error_ = String(e.what());
        return false;
    }
}
bool PixteeGameplay::cycle_club(int delta) {
    if (!hole_) return false;
    try {
        controls_.cycle_club(delta);
        last_error_ = "";
        return true;
    } catch (const std::exception& e) {
        last_error_ = String(e.what());
        return false;
    }
}
bool PixteeGameplay::meter_click_raw(int sample) {
    if (!hole_ || sample < 0 || sample > 105) return false;
    try {
        controls_.meter_click(static_cast<std::uint16_t>(sample));
        if (controls_.stage() == sensigolf::ClassicMeterStage::ShotQueued)
            return controls_.dispatch_to(*hole_);
        last_error_ = "";
        return true;
    } catch (const std::exception& e) {
        last_error_ = String(e.what());
        return false;
    }
}
void PixteeGameplay::cancel_meter() { controls_.cancel_meter(); }
bool PixteeGameplay::acknowledge_hazard_recovery() {
    if (!hole_) return false;
    try {
        hole_->acknowledge_hazard_recovery();
        controls_.sync_hole(*hole_);
        last_error_ = "";
        return true;
    } catch (const std::exception& e) {
        last_error_ = String(e.what());
        return false;
    }
}
bool PixteeGameplay::acknowledge_special_green_stop() {
    if (!hole_) return false;
    try {
        hole_->acknowledge_special_green_stop();
        controls_.sync_hole(*hole_);
        last_error_ = "";
        return true;
    } catch (const std::exception& e) {
        last_error_ = String(e.what());
        return false;
    }
}
void PixteeGameplay::step_tick() {
    if (!hole_) return;
    try {
        hole_->step(); // One authoritative recovered C++ tick: 936/65536 seconds.
        controls_.sync_hole(*hole_);
        last_error_ = "";
    } catch (const std::exception& e) {
        last_error_ = String(e.what());
    }
}
bool PixteeGameplay::step_ticks(int count) {
    // Useful for deterministic simulation tests and replay; never use
    // render-frame delta as a source of physics values.
    if (!hole_ || count < 0 || count > 4096) return false;
    for (int i = 0; i < count; ++i) {
        try {
            hole_->step();
            controls_.sync_hole(*hole_);
        } catch (const std::exception& e) {
            last_error_ = String(e.what());
            return false;
        }
    }
    last_error_ = "";
    return true;
}
Dictionary PixteeGameplay::rules_contract() const {
    Dictionary d;
    d["engine"] = "recovered_sensigolf_cpp";
    d["fixed_tick_numerator"] = 936;
    d["fixed_tick_denominator"] = 65536;
    d["heading_units"] = 4096;
    d["club_count"] = 13;
    d["putter_index"] = 12;
    d["raw_power_min"] = 0;
    d["raw_power_max"] = 105;
    d["raw_accuracy_sweet_spot"] = 63;
    d["meter_press_count"] = 3;
    d["meter_scheduler_verified"] = false; // Do not invent values.
    d["pixel_art_loaded"] = false; // Engine-only milestone.
    d["godot_physics_authoritative"] = false;
    return d;
}
Dictionary PixteeGameplay::snapshot() const {
    Dictionary d;
    d["ready"] = hole_ != nullptr;
    d["error"] = last_error_;
    if (!hole_) return d;
    const auto& b = hole_->ball_state();
    const auto phase = hole_->phase();
    const bool shot_active = phase == sensigolf::HoleSessionPhase::ShotActive;
    const std::int64_t current_x = shot_active ? b.x_raw : hole_->ball_x_raw();
    const std::int64_t current_y = shot_active ? b.y_raw : hole_->ball_y_raw();
    d["hole_phase"] = static_cast<int>(phase);
    d["meter_phase"] = static_cast<int>(controls_.stage());
    d["can_shoot"] = controls_.enabled();
    d["strokes"] = static_cast<int64_t>(hole_->strokes());
    d["tick"] = static_cast<int64_t>(b.tick);
    d["shot_phase"] = static_cast<int>(b.phase);
    d["ball_x_raw"] = current_x;
    d["ball_y_raw"] = current_y;
    d["ball_z_raw"] = static_cast<std::int64_t>(b.z_raw);
    d["velocity_x_raw"] = static_cast<std::int64_t>(b.vx_raw);
    d["velocity_y_raw"] = static_cast<std::int64_t>(b.vy_raw);
    d["velocity_z_raw"] = static_cast<std::int64_t>(b.vz_raw);
    d["ball_x"] = double(current_x) / 65536.0;
    d["ball_y"] = double(current_y) / 65536.0;
    d["ball_z"] = double(b.z_raw) / 65536.0;
    d["aim_raw"] = static_cast<int>(controls_.aim_raw());
    d["club_index"] = static_cast<int>(controls_.club_index());
    d["distance_to_cup"] = static_cast<int64_t>(hole_->distance_to_hole());
    d["green_mode"] = hole_->green_mode();
    d["hazard_pause_remaining"] = static_cast<int>(hole_->hazard_pause_remaining());
    d["special_green_pause_remaining"] = static_cast<int>(hole_->special_green_pause_remaining());
    d["hazard"] = b.hazard;
    d["holed"] = b.holed;
    d["round_complete"] = hole_->round_complete();
    d["gameplay_authority"] = "Recovered C++ ClassicHoleSession";
    if (const auto m = hole_->hole_metadata()) {
        d["hole_index"] = static_cast<int>(m->hole_index);
        d["par"] = static_cast<int>(m->par);
    }
    if (const auto next = hole_->next_hole_index())
        d["next_hole_index"] = static_cast<int>(*next);
    if (const auto unsupported = hole_->unsupported_descriptor())
        d["unsupported_descriptor"] = static_cast<int>(*unsupported);
    const auto& counters = hole_->recovered_counters();
    d["recovered_stroke_counter_52"] = static_cast<int>(counters.player_52);
    d["recovered_stroke_counter_56"] = static_cast<int>(counters.player_56);
    d["recovered_par_counter_58"] = static_cast<int>(counters.player_58);
    d["recovered_score_counter_48"] = static_cast<int>(counters.player_48);
    try {
        const auto surface = hole_->current_surface();
        d["terrain_descriptor"] = static_cast<int>(surface.descriptor_index);
        d["surface_code"] = static_cast<int>(surface.landing_code);
        d["lie_profile_slot"] = static_cast<int>(surface.profile_slot);
        d["slope_direction"] = static_cast<int>(surface.slope_direction);
        d["slope_magnitude"] = static_cast<int>(surface.slope_magnitude);
        d["surface_supported"] = surface.product_supported;
    } catch (const std::exception& e) {
        // Display layer must not crash if a gap in authored terrain is found.
        d["surface_error"] = String(e.what());
    }
    return d;
}
} // namespace godot
