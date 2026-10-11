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
}
