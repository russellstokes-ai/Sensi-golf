#include <cassert>

#include "sensigolf/recovered_hazard_recovery.hpp"

using namespace sensigolf::recovered;

int main() {
    HazardRecoveryInput in{};
    in.ball_x=50<<16;
    in.ball_y=60<<16;
    in.safe_anchor_x=85<<16;
    in.safe_anchor_y=95<<16;
    in.x_extent=100;
    in.y_extent=100;

    auto a=recover_hazard_position(in);
    assert(!a.used_safe_anchor);
    assert(a.ball_x==(50<<16));
    assert(a.ball_y==(60<<16));
    assert(a.player_x==(35<<16));
    assert(a.player_y==(45<<16));
    assert(a.height==0);
    assert(a.pause==60);
    assert(a.player_flags==0);

    in.ball_x=500<<16;
    in.ball_y=600<<16;
    auto b=recover_hazard_position(in);
    assert(b.used_safe_anchor);
    assert(b.ball_x==(100<<16));
    assert(b.ball_y==(110<<16));
    assert(b.player_x==(85<<16));
    assert(b.player_y==(95<<16));
    assert(b.pause==60);
    return 0;
}
