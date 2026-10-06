#include <cassert>

#include "sensigolf/recovered_flight.hpp"
#include "sensigolf/recovered_ground.hpp"

using namespace sensigolf::recovered;

int main() {
    LaunchInput input{};
    input.club_index = 0;
    input.lie_index = 0;
    input.captured_power = 105;
    input.accuracy_tick = 63;
    input.player_direction = 0;

    auto state = launch_normal_shot(input);
    int first_contact = -1;
    int rest_tick = -1;

    for (int tick = 1; tick <= 512; ++tick) {
        const auto step = step_generic_flat_surface(state);
        if (step.contacted_ground && first_contact < 0) {
            first_contact = tick;
        }
        if (step.resting) {
            rest_tick = tick;
            break;
        }
    }

    assert(first_contact == 80);
    assert(rest_tick == 152);
    assert(state.height == 0);
    assert(state.vertical_force == 0);
    assert(state.horizontal_force == 0);

    // Landing code 0x23 is the original immediate-stop hazard branch used by
    // WATER/NO GO/OUT OF BOUNDS descriptors. The same shot must stop on its
    // first contact rather than enter bounce/roll.
    state = launch_normal_shot(input);
    first_contact = -1;
    rest_tick = -1;
    bool hazard_stop = false;
    for (int tick = 1; tick <= 512; ++tick) {
        const auto step = step_controlled_surface(state, 0x23);
        if (step.contacted_ground && first_contact < 0) {
            first_contact = tick;
        }
        if (step.hazard_stop) {
            hazard_stop = true;
        }
        if (step.resting) {
            rest_tick = tick;
            break;
        }
    }
    assert(first_contact == 80);
    assert(rest_tick == 80);
    assert(hazard_stop);
    assert(state.height == 0);
    assert(state.vertical_force == 0);
    assert(state.horizontal_force == 0);
    return 0;
}
