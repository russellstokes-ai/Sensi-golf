#include <cassert>
#include <cstdint>
#include "sensigolf/recovered_world_boundary.hpp"
using namespace sensigolf::recovered;
int main() {
    OriginalWorldBoundaryInput in{};
    in.x_extent=34*16-0x100;
    in.y_extent=114*8-0xD0;
    in.player_flags=0x8002;
    in.strokes_on_hole=2;
    in.strokes_total=5;
    in.ball_pause=13;
    in.game_gate=3;
    auto a=original_world_boundary_check(in);
    assert(!a.outside_world && a.source_map_ax==0);
    assert(!a.requires_out_of_bounds_dispatch && a.strokes_total==5);
    in.ball_x=(34*16-1)<<16;
    in.ball_y=(114*8-1)<<16;
    a=original_world_boundary_check(in);
    assert(!a.outside_world);
    in.ball_x=34*16<<16;
    a=original_world_boundary_check(in);
    assert(a.outside_world && a.source_map_ax==-1);
    assert(a.requires_out_of_bounds_dispatch && a.player_flags==4);
    assert(a.strokes_on_hole==3 && a.strokes_total==6);
    assert(a.ball_pause==100 && a.game_gate==1);
    in.ball_x=0;
    in.ball_y=114*8<<16;
    assert(original_world_boundary_check(in).outside_world);
    in.ball_y=-(1<<16);
    assert(original_world_boundary_check(in).outside_world);
    in.ball_x=5<<16;
    in.ball_y=100<<16;
    in.strokes_total=65535;
    a=original_world_boundary_check(in);
    assert(!a.outside_world && a.strokes_total==65535);
    in.ball_x=-(1<<16);
    a=original_world_boundary_check(in);
    assert(a.outside_world && a.strokes_total==0);
    // Original bit2/0x40AACF post-event recovery must retain safe-anchor
    // coordinates and clear the intermediate source status word.
    OriginalWorldOobRestoreInput restore{};
    restore.coordinates.ball_x=544<<16;
    restore.coordinates.ball_y=100<<16;
    restore.coordinates.safe_anchor_x=48<<16;
    restore.coordinates.safe_anchor_y=85<<16;
    restore.coordinates.x_extent=34*16-256;
    restore.coordinates.y_extent=114*8-208;
    restore.player_mode_5a=5;
    restore.player_mode_5c=0xffff;
    restore.player_mode_62=32;
    auto restored=original_world_oob_restore(restore);
    assert(restored.relocated.used_safe_anchor);
    assert(restored.relocated.ball_x==(63<<16));
    assert(restored.relocated.ball_y==(100<<16));
    assert(restored.relocated.player_x==(48<<16));
    assert(restored.relocated.player_y==(85<<16));
    assert(restored.relocated.player_flags==0);
    assert(restored.relocated.pause==60);
    assert(restored.player_mode_5c==0);
    assert(restored.player_mode_62==1);
    restore.player_mode_5a=4;
    restored=original_world_oob_restore(restore);
    assert(restored.player_mode_62==32);
    restore.player_mode_5a=0xffff; // signed -1 <= 4
    restored=original_world_oob_restore(restore);
    assert(restored.player_mode_62==32);

    const auto event=original_world_oob_event();
    assert(event.byte_420fef==1);
    assert(event.dword_420544==0x46);
    assert(event.dword_420548==0x1000d);
    assert(event.source_esi==0x420670u);

}
