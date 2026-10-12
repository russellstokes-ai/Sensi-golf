#pragma once
#include <cstdint>
#include <memory>
#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/string.hpp"
#include "pixtee_multi_round_runtime.hpp"

namespace godot {
// An independently instantiable Godot native multiplayer host. Each player
// owns exactly one recovered C++ physics/round/PRNG session. No duplicate
// ball or scoring maths in GDScript; no fabricated tournament winner.
class PixteeMultiRoundBridge : public RefCounted {
    GDCLASS(PixteeMultiRoundBridge, RefCounted)
protected:
    static void _bind_methods();
public:
    bool begin_multi_round(int player_count,
        const PackedInt32Array& original_18_resource_ids,
        const PackedInt32Array& par_by_resource_100,
        const PackedInt32Array& seed0_by_player,
        const PackedInt32Array& seed1_by_player);
    bool register_course_resource(int id,
        const PackedByteArray& mapm,const PackedByteArray& spt,
        const PackedByteArray& descriptor,const PackedByteArray& selector,
        const PackedByteArray& maps);
    bool load_next_hole_for_everyone();
    bool propose_original_turn(int forced_holder,
        const PackedInt32Array& priorities,
        const PackedInt32Array& eliminated,
        const PackedInt32Array& identities);
    bool propose_original_competition_turn(
        int mode,int active_side,int ball0_distance,int ball1_distance,
        int side0_flag,int side1_flag,
        const PackedInt32Array& slot_bytes,
        const PackedInt32Array& previous_slots);
    int acknowledge_original_turn();
    bool set_club_index(int club);
    bool meter_click_raw(int sample);
    bool start_original_welly();
    bool tick_original_welly(bool pressed);
    void step_tick();
    bool acknowledge_hazard();
    bool acknowledge_special_green();
    bool commit_scored_hole();
    Dictionary snapshot() const;
    String last_error() const {return last_error_;}
private:
    std::unique_ptr<pixtee::PixteeMultiRoundRuntime> game_{};
    String last_error_{};
};
} // namespace godot
