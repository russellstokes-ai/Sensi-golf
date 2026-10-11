#include "pixtee_gameplay.hpp"
#include "pixtee_course_adapter.hpp"
#include <cstddef>
#include <exception>
#include "godot_cpp/core/class_db.hpp"

namespace godot {
void PixteeGameplay::_bind_methods() {
    ClassDB::bind_method(D_METHOD("load_lakewood_hole1"),
                         &PixteeGameplay::load_lakewood_hole1);
    ClassDB::bind_method(D_METHOD("set_aim_raw", "heading"),
                         &PixteeGameplay::set_aim_raw);
    ClassDB::bind_method(D_METHOD("set_club_index", "index"),
                         &PixteeGameplay::set_club_index);
    ClassDB::bind_method(D_METHOD("meter_click_raw", "sample"),
                         &PixteeGameplay::meter_click_raw);
    ClassDB::bind_method(D_METHOD("cancel_meter"), &PixteeGameplay::cancel_meter);
    ClassDB::bind_method(D_METHOD("step_tick"), &PixteeGameplay::step_tick);
    ClassDB::bind_method(D_METHOD("snapshot"), &PixteeGameplay::snapshot);
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
            *course_, std::size_t{0},
            sensigolf::ClassicHoleMetadata{0,4,0});
        controls_.sync_hole(*hole_);
        return true;
    } catch (const std::exception& e) {
        last_error_ = String(e.what());
        hole_.reset();
        course_.reset();
        return false;
    }
}
bool PixteeGameplay::set_aim_raw(int raw) {
    if (!hole_) return false;
    try {
        if (raw < 0 || raw > 4095) return false;
        controls_.set_aim_raw(static_cast<std::uint16_t>(raw));
        return true;
    } catch (const std::exception& e) {
        last_error_ = String(e.what());
        return false;
    }
}
bool PixteeGameplay::set_club_index(int index) {
    if (!hole_) return false;
    try {
        if (index < 0 || index > 12) return false;
        controls_.set_club(static_cast<std::uint16_t>(index));
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
        return true;
    } catch (const std::exception& e) {
        last_error_ = String(e.what());
        return false;
    }
}
void PixteeGameplay::cancel_meter() {
    controls_.cancel_meter();
}
void PixteeGameplay::step_tick() {
    if (!hole_) return;
    try {
        hole_->step(); // ONE authoritative core update. No Godot physics.
        if (hole_->phase() == sensigolf::HoleSessionPhase::ReadyForShot)
            controls_.sync_hole(*hole_);
    } catch (const std::exception& e) {
        last_error_ = String(e.what());
    }
}
Dictionary PixteeGameplay::snapshot() const {
    Dictionary d;
    d["ready"] = hole_ != nullptr;
    d["error"] = last_error_;
    if (!hole_) return d;
    const auto& b = hole_->ball_state();
    d["hole_phase"] = static_cast<int>(hole_->phase());
    d["meter_phase"] = static_cast<int>(controls_.stage());
    d["strokes"] = static_cast<int64_t>(hole_->strokes());
    d["tick"] = static_cast<int64_t>(b.tick);
    d["ball_x_raw"] = static_cast<int64_t>(b.x_raw);
    d["ball_y_raw"] = static_cast<int64_t>(b.y_raw);
    d["ball_z_raw"] = static_cast<int64_t>(b.z_raw);
    d["ball_x"] = double(b.x_raw) / 65536.0;
    d["ball_y"] = double(b.y_raw) / 65536.0;
    d["ball_z"] = double(b.z_raw) / 65536.0;
    d["aim_raw"] = static_cast<int>(controls_.aim_raw());
    d["club_index"] = static_cast<int>(controls_.club_index());
    d["surface_code"] = static_cast<int>(hole_->current_surface().landing_code);
    d["distance_to_cup"] = static_cast<int64_t>(hole_->distance_to_hole());
    d["hazard"] = b.hazard;
    d["holed"] = b.holed;
    d["gameplay_authority"] = "Recovered C++ ClassicHoleSession";
    return d;
}
} // namespace godot
