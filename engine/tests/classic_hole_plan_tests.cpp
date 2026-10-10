#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>

#include "sensigolf/classic_hole_plan.hpp"

using namespace sensigolf;

int main() {
    std::array<std::uint8_t, kClassicRoundHoleCount> order{};
    for (std::size_t i = 0; i < order.size(); ++i) {
        order[i] = static_cast<std::uint8_t>(i + 1u);
    }

    std::array<std::uint8_t, kClassicParTableSize> pars{};
    for (std::uint8_t id = 1; id <= 18; ++id) {
        pars[id] = static_cast<std::uint8_t>(3u + (id % 3u));
    }

    ClassicHolePlan plan(order, pars);

    const auto first = plan.hole(0);
    assert(first.round_index == 0u);
    assert(first.resource_id == 1u);
    assert(first.par == 4u);
    assert(first.resources.mapm_map == "mapm01.map");
    assert(first.resources.maps_map == "maps01.map");
    assert(first.resources.mapm_spt == "mapm01.spt");

    const auto last = plan.hole(17);
    assert(last.round_index == 17u);
    assert(last.resource_id == 18u);
    assert(last.resources.mapm_map == "mapm18.map");
    assert(last.resources.maps_map == "maps18.map");
    assert(last.resources.mapm_spt == "mapm18.spt");

    const auto seventy_two = ClassicHolePlan::resource_names(72);
    assert(seventy_two.mapm_map == "mapm72.map");
    assert(seventy_two.maps_map == "maps72.map");
    assert(seventy_two.mapm_spt == "mapm72.spt");

    bool threw = false;
    try {
        (void)plan.hole(18);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    assert(threw);

    auto bad_order = order;
    bad_order[4] = 0;
    threw = false;
    try {
        ClassicHolePlan bad(bad_order, pars);
        (void)bad;
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    auto missing_pars = pars;
    missing_pars[7] = 0;
    threw = false;
    try {
        ClassicHolePlan bad(order, missing_pars);
        (void)bad;
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    return 0;
}
