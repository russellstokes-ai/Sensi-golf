#include <cassert>
#include <cstdint>
#include <stdexcept>

#include "sensigolf/recovered_flight.hpp"

using namespace sensigolf::recovered;

int main() {
    const auto& clubs = club_launch_parameters();
    assert(clubs.size() == 13);
    assert(clubs[0].vertical_base == 8192);
    assert(clubs[0].horizontal_base == 81920);
    assert(clubs[0].power_scale == 3168);
    assert(clubs[12].special_putter_path);
    assert(clubs[12].vertical_base == 0);

    assert(trig_q14(0) == 0);
    assert(trig_q14(1) == 25);
    assert(trig_q14(256) == 6269);
    assert(trig_q14(512) == 11585);
    assert(trig_q14(768) == 15136);
    assert(trig_q14(1024) == 16384);
    assert(trig_q14(2048) == 0);
    assert(trig_q14(3072) == -16383);
    assert(trig_q14(4095) == -25);

    LaunchInput launch{};
    launch.club_index = 0;
    launch.captured_power = 105;
    launch.swing_adjuster = 0;
    launch.player_direction = 0;

    auto state = launch_normal_shot(launch);
    assert(state.vertical_force == 340832);
    assert(state.horizontal_force == 414560);
    assert(state.direction == 0);

    const auto result = step_clear_air(state);
    assert(result == AirborneStepResult::Airborne);
    assert(state.vertical_force == 332384);
    assert(state.height == 332384);
    assert(state.horizontal_force == 410720);
    assert(state.direction == 0);
    assert(state.x == 0);
    assert(state.y == 410720);

    launch.captured_power = 100;
    launch.player_direction = 1000;
    launch.swing_adjuster = 1;
    state = launch_normal_shot(launch);
    assert(state.direction == 1000);
    assert(state.vertical_force == 8192 + 3168 * 100);
    assert(state.horizontal_force == 81920 + 3168 * 100);
    assert(step_clear_air(state) == AirborneStepResult::Airborne);
    assert(state.direction == 998);

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
