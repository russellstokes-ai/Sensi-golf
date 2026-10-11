#include <cassert>
#include <stdexcept>

#include "sensigolf/classic_terrain.hpp"

using namespace sensigolf;

int main() {
    assert(kClassicTerrainDescriptorCount == 77);

    const auto& skirt = classic_terrain_descriptor(0);
    assert(skirt.landing_code == 2);
    assert(skirt.profile_slot == 5);
    assert(skirt.name == "SKIRT");
    assert(skirt.product_supported);

    const auto& water = classic_terrain_descriptor(6);
    assert(water.landing_code == 35);
    assert(water.profile_slot == 7);
    assert(water.product_supported);

    const auto& hole = classic_terrain_descriptor(7);
    assert(hole.landing_code == 8);
    assert(hole.variant == -5);
    assert(hole.profile_slot == 9);
    assert(hole.product_supported);

    const auto& green_h3 = classic_terrain_descriptor(23);
    assert(green_h3.landing_code == 9);
    assert(!green_h3.product_supported);

    const auto& green_h4 = classic_terrain_descriptor(31);
    assert(green_h4.landing_code == 1);
    assert(green_h4.profile_slot == 6);
    assert(green_h4.product_supported);

    const auto& mud = classic_terrain_descriptor(72);
    assert(mud.landing_code == 60);
    assert(!mud.product_supported);

    bool threw = false;
    try {
        (void)classic_terrain_descriptor(77);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    assert(threw);

    assert(classic_landing_code_product_supported(1));
    assert(classic_landing_code_product_supported(8));
    assert(classic_landing_code_product_supported(35));
    assert(!classic_landing_code_product_supported(9));
    assert(!classic_landing_code_product_supported(10));
    assert(!classic_landing_code_product_supported(50));
    assert(!classic_landing_code_product_supported(60));

    // Source descriptor names retain a distinct OOB label even though all
    // these cells use the same original numeric landing code 35.
    assert(classic_named_hazard(6)==ClassicNamedHazard::Water);
    assert(classic_named_hazard(14)==ClassicNamedHazard::NoGo);
    assert(classic_named_hazard(22)==ClassicNamedHazard::OutOfBoundsTile);
    assert(classic_named_hazard(30)==ClassicNamedHazard::OtherCode35);
    assert(classic_named_hazard(38)==ClassicNamedHazard::OtherCode35);
    for(const auto i: {46,54,62,70})
        assert(classic_named_hazard(i)==ClassicNamedHazard::Water);
    assert(classic_named_hazard(0)==ClassicNamedHazard::None);
    assert(classic_named_hazard(47)==ClassicNamedHazard::None);

    return 0;
}
