#include <cassert>
#include <stdexcept>

#include "sensigolf/recovered_flight.hpp"

using namespace sensigolf::recovered;

int main() {
    const auto& clubs = club_launch_parameters();
    assert(clubs.size() == 13);
    assert(clubs[0].vertical_base == 8192);
    assert(clubs[12].special_putter_path);

    assert(trig_q14(0) == 0);
    assert(trig_q14(1024) == 16384);
    assert(trig_q14(3072) == -16383);

    // Club 0, lie 0 -> profile 1, centre accuracy.
    LaunchInput launch{};
    launch.club_index = 0;
    launch.lie_index = 0;
    launch.captured_power = 105;
    launch.accuracy_tick = 63;
    launch.player_direction = 0;
    auto state = launch_normal_shot(launch);
    assert(state.adjusted_power == 105);
    assert(state.swing_adjuster == 0);
    assert(state.direction == 0);
    assert(state.vertical_force == 340832);
    assert(state.horizontal_force == 414560);

    assert(step_clear_air(state) == AirborneStepResult::Airborne);
    assert(state.vertical_force == 332384);
    assert(state.height == 332384);
    assert(state.horizontal_force == 410720);
    assert(state.x == 0);
    assert(state.y == 410720);

    // Club 5, lie 2 -> profile 5. Tick 59 => error -4, swing -1.
    launch.club_index = 5;
    launch.lie_index = 2;
    launch.captured_power = 83;
    launch.accuracy_tick = 59;
    launch.player_direction = 777;
    state = launch_normal_shot(launch);
    assert(state.adjusted_power == 79);
    assert(state.swing_adjuster == -1);
    assert(state.direction == 841);
    assert(state.vertical_force == 299200);
    assert(state.horizontal_force == 276672);
    assert(step_clear_air(state) == AirborneStepResult::Airborne);
    assert(state.direction == 843);

    // Club 11, lie 0 -> profile 6. Tick 67 => error +4, swing +1.
    launch.club_index = 11;
    launch.lie_index = 0;
    launch.captured_power = 60;
    launch.accuracy_tick = 67;
    launch.player_direction = 3072;
    state = launch_normal_shot(launch);
    assert(state.adjusted_power == 56);
    assert(state.swing_adjuster == 1);
    assert(state.direction == 3008);
    assert(state.vertical_force == 249344);
    assert(state.horizontal_force == 159232);

    bool out_of_bounds_rejected = false;
    try {
        launch.accuracy_tick = 80;
        (void)launch_normal_shot(launch);
    } catch (const std::out_of_range&) {
        out_of_bounds_rejected = true;
    }
    assert(out_of_bounds_rejected);

    return 0;
}
