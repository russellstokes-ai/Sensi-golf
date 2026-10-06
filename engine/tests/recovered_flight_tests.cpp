#include <cassert>
#include <cstdint>
#include <stdexcept>

#include "sensigolf/recovered_flight.hpp"

using namespace sensigolf::recovered;

int main() {
    const auto& clubs = club_launch_parameters();
    assert(clubs.size() == 13);

    // Original index 0 has the lowest launch and greatest horizontal base.
    assert(clubs[0].vertical_base == 8192);
    assert(clubs[0].horizontal_base == 81920);
    assert(clubs[0].power_scale == 3168);

    // Original index 12 is a distinct putter path.
    assert(clubs[12].special_putter_path);
    assert(clubs[12].vertical_base == 0);

    // Recovered Q14 sine table anchor values.
    assert(trig_q14(0) == 0);
    assert(trig_q14(1) == 25);
    assert(trig_q14(256) == 6269);
    assert(trig_q14(512) == 11585);
    assert(trig_q14(768) == 15136);
    assert(trig_q14(1024) == 16384);
    assert(trig_q14(2048) == 0);
    assert(trig_q14(3072) == -16383);
    assert(trig_q14(4095) == -25);

    // Full-power straight index-0 launch: P=105, accuracy error=0.
    LaunchInput launch{};
    launch.club_index = 0;
    launch.captured_power = 105;
    launch.accuracy_error = 0;
    launch.swing_adjuster = 0; // every recovered profile centre is exactly zero
    launch.player_direction = 0;

    auto state = launch_normal_shot(launch);
    assert(state.vertical_force == 340832);   // 8192 + 3168*105
    assert(state.horizontal_force == 414560); // 81920 + 3168*105
    assert(state.direction == 0);

    // First clear-air tick matches the recovered integer update path.
    const auto result = step_clear_air(state);
    assert(result == AirborneStepResult::Airborne);
    assert(state.vertical_force == 332384);   // -0x2100
    assert(state.height == 332384);
    assert(state.horizontal_force == 410720); // -0x0F00
    assert(state.direction == 0);
    assert(state.x == 0);                     // sin(0)
    assert(state.y == 410720);                // sin(pi/2) == 1 in Q14

    // Accuracy error changes initial heading and reduces captured power.
    launch.captured_power = 100;
    launch.accuracy_error = 3;
    launch.player_direction = 1000;
    launch.swing_adjuster = 1;
    state = launch_normal_shot(launch);
    assert(state.direction == ((1000 - 48) & 0x0FFF));
    assert(state.vertical_force == 8192 + 3168 * 97);
    assert(state.horizontal_force == 81920 + 3168 * 97);

    bool putter_rejected = false;
    try {
        launch.club_index = 12;
        (void)launch_normal_shot(launch);
    } catch (const std::invalid_argument&) {
        putter_rejected = true;
    }
    assert(putter_rejected);

    return 0;
}
