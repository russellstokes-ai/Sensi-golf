#pragma once
#include <memory>
#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/string.hpp"
#include "sensigolf/classic_course_resources.hpp"
#include "sensigolf/classic_hole_session.hpp"
#include "sensigolf/classic_mobile_controls.hpp"

namespace godot {
class PixteeGameplay : public RefCounted {
    GDCLASS(PixteeGameplay, RefCounted);
protected:
    static void _bind_methods();
public:
    PixteeGameplay() = default;
    bool load_lakewood_hole1();
    bool set_aim_raw(int value);
    bool set_club_index(int value);
    bool meter_click_raw(int value);
    void cancel_meter();
    void step_tick();
    Dictionary snapshot() const;
    String last_error() const { return last_error_; }
private:
    std::unique_ptr<sensigolf::ClassicCourseResources> course_;
    std::unique_ptr<sensigolf::ClassicHoleSession> hole_;
    sensigolf::ClassicMobileControls controls_;
    String last_error_;
};
}
