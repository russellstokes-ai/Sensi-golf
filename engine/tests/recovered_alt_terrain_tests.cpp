#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <initializer_list>
#include "sensigolf/recovered_alt_terrain.hpp"
using namespace sensigolf::recovered;
int main() {
    for(auto code:{35u,60u}) {
        auto o=original_alternate_terrain_contact(
            static_cast<std::uint16_t>(code),-8448,50000);
        assert(o.route==OriginalAlternateTerrainRoute::ReturnZero);
        assert(o.vertical_force==-8448 && o.horizontal_force==50000);
    }
    for(auto code:{8u,9u,10u}) {
        auto o=original_alternate_terrain_contact(
            static_cast<std::uint16_t>(code),-8448,50000);
        assert(o.route==OriginalAlternateTerrainRoute::ReturnMinusOne);
        assert(o.vertical_force==-8448 && o.horizontal_force==50000);
    }
    auto o=original_alternate_terrain_contact(7,-8448,50000);
    assert(o.route==OriginalAlternateTerrainRoute::ContinueGround);
    assert(o.vertical_force==4224 && o.horizontal_force==52112);
    o=original_alternate_terrain_contact(0,32768,1);
    assert(o.route==OriginalAlternateTerrainRoute::ReturnZero);
    // Original rebound would give -16384 V, but the source JG fails
    // because the signed horizontal sum is negative; both forces zero.
    assert(o.vertical_force==0 && o.horizontal_force==0);
    o=original_alternate_terrain_contact(65535,-8448,50000);
    assert(o.route==OriginalAlternateTerrainRoute::ContinueGround);
}
