#pragma once
#include <memory>
#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/string.hpp"
#include "pixtee_round_runtime.hpp"

namespace godot {
// ENGINE-ONLY Godot boundary. The native PixteeRoundRuntime contains the
// original-rule ClassicGameSession + ClassicReplayJournal; never rewrite
// scores, course collisions, spin or hazard transitions in GDScript.
class PixteeRoundBridge : public RefCounted {
    GDCLASS(PixteeRoundBridge, RefCounted)
protected:
    static void _bind_methods();
public:
    // Pure original v1.014 next-player decision; caller supplies the
    // original per-player priority/eligibility state, not invented metrics.
    Dictionary original_turn_score_adjust(
        int player_count, const PackedInt32Array& strokes,
        const PackedInt32Array& totals,
        const PackedInt32Array& completed,
        const PackedInt32Array& ball_distances);
    Dictionary original_cpu_shot_variation(
        bool bypass, int power, int variation,
        int random100, int random_power, int random_direction);
    Dictionary original_cpu_search_parameters(int original_mode, bool special_flag);
    int original_cpu_candidate_choice(
        const PackedInt32Array& primary_status,
        const PackedInt32Array& secondary_status,
        const PackedInt32Array& candidate_metrics);
    int original_next_player_choice(
        int player_count, int forced_holder,
        const PackedInt32Array& priority,
        const PackedInt32Array& eliminated,
        const PackedInt32Array& identities);
    bool begin_round(const PackedInt32Array& order,
                     const PackedInt32Array& par_by_resource,
                     int seed0,int seed1);
    bool register_course_resource(int resource_id,
        const PackedByteArray& mapm,const PackedByteArray& spt,
        const PackedByteArray& mapi_descriptors,
        const PackedByteArray& mapi_selectors,
        const PackedByteArray& maps);
    bool load_next_hole();
    bool set_aim_raw(int heading);
    bool nudge_aim_raw(int delta);
    bool set_club_index(int index);
    bool cycle_club(int delta);
    bool meter_click_raw(int sample);
    bool start_original_welly();
    bool tick_original_welly(bool pressed);
    void cancel_meter();
    bool acknowledge_hazard();
    bool acknowledge_special_green();
    bool commit_scored_hole();
    bool step_ticks(int count);
    void step_tick();
    String save_stable_replay();
    bool restore_replay(const String& payload);
    Dictionary snapshot() const;
    Dictionary requested_course() const;
    String last_error() const {return last_error_;}
private:
    std::unique_ptr<pixtee::PixteeRoundRuntime> round_;
    String last_error_;
};
} // namespace godot
