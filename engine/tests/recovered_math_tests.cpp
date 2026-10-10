#include <cassert>
#include <cstdint>

#include "sensigolf/recovered_math.hpp"

int main() {
    using namespace sensigolf::recovered;

    assert(kDirectionMask == 0x0FFFu);
    assert(kDropPowerMax == 105u);
    assert(kGravityPerTick == 0x2100);
    assert(kNormalRollDrag == 0x0F00);
    assert(kGreenRollDrag == 0x0780);

    // Exact recovered Q14 table landmarks.
    assert(sin_q14(0) == 0);
    assert(sin_q14(1) == 25);
    assert(sin_q14(256) == 6269);
    assert(sin_q14(512) == 11585);
    assert(sin_q14(1024) == 16384);
    assert(sin_q14(2048) == 0);
    assert(sin_q14(3072) == -16383);
    assert(sin_q14(4095) == -25);
    assert(sin_q14(4096) == 0);
    assert(cos_q14(0) == 16384);
    assert(cos_q14(1024) == 0);

    assert(wrap_direction(4096) == 0);
    assert(wrap_direction(-1) == 4095);

    assert(kClubPhysics.size() == 13);
    assert(kClubPhysics[0].loaded_vertical_base() == 8192);
    assert(kClubPhysics[0].loaded_horizontal_base() == 81920);
    assert(kClubPhysics[0].power_scale == 3168);
    assert(kClubPhysics[kPutterClubIndex].loaded_vertical_base() == 0);
    assert(kClubPhysics[kPutterClubIndex].loaded_horizontal_base() == 8192);
    assert(kClubPhysics[kPutterClubIndex].power_scale == 1536);

    {
        const auto launch = make_launch_state(0, 10, 100, 3);
        assert(launch.vertical_force == 8192 + 31680);
        assert(launch.horizontal_force == 81920 + 31680);
        assert(launch.direction == 94);
    }
    {
        const auto launch = make_launch_state(0, kDropPowerMax, 0, 0);
        assert(launch.vertical_force == 8192 + 3168 * 105);
        assert(launch.horizontal_force == 81920 + 3168 * 105);
        assert(launch.direction == 0);
    }
    {
        const auto launch = make_launch_state(12, 0, 0, 1);
        assert(launch.vertical_force == 0);
        assert(launch.horizontal_force == 8192);
        assert(launch.direction == 4094);
    }

    {
        const auto [dx, dy] = project_horizontal(16384, 0);
        assert(dx == 0);
        assert(dy == 16384);
    }
    {
        const auto [dx, dy] = project_horizontal(16384, 1024);
        assert(dx == 16384);
        assert(dy == 0);
    }
    {
        const auto [dx, dy] = project_horizontal(16384, 3072);
        assert(dx == -16383);
        assert(dy == 0);
    }

    {
        RecoveredBallState ball{};
        ball.vertical_force = 20000;
        ball.height = 1000;
        apply_airborne_vertical_tick(ball);
        assert(ball.vertical_force == 11552);
        assert(ball.height == 12552);

        ball.vertical_force = -1000;
        ball.height = 100;
        apply_airborne_vertical_tick(ball);
        assert(ball.vertical_force == -9448);
        assert(ball.height == 0);
    }

    {
        RecoveredBallState ball{};
        ball.horizontal_force = 10000;
        apply_roll_drag(ball, false);
        assert(ball.horizontal_force == 6160);
        apply_roll_drag(ball, true);
        assert(ball.horizontal_force == 4240);
    }

    {
        RecoveredBallState ball{};
        ball.vertical_force = -10000;
        ball.horizontal_force = 20000;
        apply_bounce(ball);
        assert(ball.vertical_force == 5000);
        assert(ball.horizontal_force == 22500);
    }

    {
        RecoveredBallState ball{};
        ball.horizontal_force = 16384;
        ball.direction = 0;
        apply_horizontal_tick(ball);
        assert(ball.x == 0);
        assert(ball.y == 16384);
    }

    return 0;
}
