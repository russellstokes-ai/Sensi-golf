#include "sensigolf/classic_terrain.hpp"

#include <array>
#include <stdexcept>

namespace sensigolf {
namespace {

constexpr bool supported(std::uint16_t landing_code) {
    return landing_code == 1
        || (landing_code >= 2 && landing_code <= 8)
        || landing_code == 0x23;
}

constexpr std::array<ClassicTerrainDescriptor, kClassicTerrainDescriptorCount>
kTerrain{{
    {2,0,5,"SKIRT",supported(2)},
    {3,0,4,"FAIRWAY",supported(3)},
    {4,0,3,"SEMI ROUGH",supported(4)},
    {5,0,2,"ROUGH",supported(5)},
    {6,0,1,"V ROUGH",supported(6)},
    {7,0,0,"SAND",supported(7)},
    {35,0,7,"WATER",supported(35)},
    {8,-5,9,"GREEN H1",supported(8)},
    {2,1,5,"SKIRT H1",supported(2)},
    {3,1,4,"FAIRWAY H1",supported(3)},
    {4,1,3,"SEMI ROUGH H1",supported(4)},
    {5,1,2,"ROUGH H1",supported(5)},
    {6,1,1,"V ROUGH H1",supported(6)},
    {7,1,0,"SAND H1",supported(7)},
    {35,1,7,"NO GO",supported(35)},
    {10,0,0,"GREEN H2",supported(10)},
    {2,2,5,"SKIRT H2",supported(2)},
    {3,2,4,"FAIRWAY H2",supported(3)},
    {4,2,3,"SEMI ROUGH H2",supported(4)},
    {5,2,2,"ROUGH H2",supported(5)},
    {6,2,1,"V ROUGH H2",supported(6)},
    {7,2,0,"SAND H2",supported(7)},
    {35,2,7,"OUT OF BOUNDS",supported(35)},
    {9,0,0,"GREEN H3",supported(9)},
    {2,3,5,"SKIRT H3",supported(2)},
    {3,3,4,"FAIRWAY H3",supported(3)},
    {4,3,3,"SEMI ROUGH H3",supported(4)},
    {5,3,2,"ROUGH H3",supported(5)},
    {6,3,1,"V ROUGH H3",supported(6)},
    {7,3,0,"SAND H3",supported(7)},
    {35,3,7,"TARMAC",supported(35)},
    {1,0,6,"GREEN H4",supported(1)},
    {2,4,5,"SKIRT H4",supported(2)},
    {3,4,4,"FAIRWAY H4",supported(3)},
    {4,4,3,"SEMI ROUGH H4",supported(4)},
    {5,4,2,"ROUGH H4",supported(5)},
    {6,4,1,"V ROUGH H4",supported(6)},
    {7,4,0,"SAND H4",supported(7)},
    {35,4,7,"DIVOT",supported(35)},
    {6,-1,1,"GREEN D1",supported(6)},
    {2,-1,5,"SKIRT D1",supported(2)},
    {3,-1,4,"FAIRWAY D1",supported(3)},
    {4,-1,3,"SEMI ROUGH D1",supported(4)},
    {5,-1,2,"ROUGH D1",supported(5)},
    {6,-1,1,"V ROUGH D1",supported(6)},
    {7,-1,0,"SAND D1",supported(7)},
    {35,-1,7,"WATER D1",supported(35)},
    {50,-1,9,"GREEN D2",supported(50)},
    {2,-2,5,"SKIRT D2",supported(2)},
    {3,-2,4,"FAIRWAY D2",supported(3)},
    {4,-2,3,"SEMI ROUGH D2",supported(4)},
    {5,-2,2,"ROUGH D2",supported(5)},
    {6,-2,1,"V ROUGH D2",supported(6)},
    {7,-2,0,"SAND D2",supported(7)},
    {35,-2,7,"WATER D2",supported(35)},
    {50,-2,9,"GREEN D3",supported(50)},
    {2,-3,5,"SKIRT D3",supported(2)},
    {3,-3,4,"FAIRWAY D3",supported(3)},
    {4,-3,3,"SEMI ROUGH D3",supported(4)},
    {5,-3,2,"ROUGH D3",supported(5)},
    {6,-3,1,"V ROUGH D3",supported(6)},
    {7,-3,0,"SAND D3",supported(7)},
    {35,-3,7,"WATER D3",supported(35)},
    {50,-3,9,"GREEN D4",supported(50)},
    {2,-4,5,"SKIRT D4",supported(2)},
    {3,-4,4,"FAIRWAY D4",supported(3)},
    {4,-4,3,"SEMI ROUGH D4",supported(4)},
    {5,-4,2,"ROUGH D4",supported(5)},
    {6,-4,1,"V ROUGH D4",supported(6)},
    {7,-4,0,"SAND D4",supported(7)},
    {35,-4,7,"WATER D4",supported(35)},
    {50,-4,9,"MUD D1",supported(50)},
    {60,-1,8,"MUD D2",supported(60)},
    {60,-2,8,"MUD D3",supported(60)},
    {60,-3,8,"MUD D4",supported(60)},
    {60,-4,8,"MUD D5",supported(60)},
    {60,-5,8,"UNUSED",supported(60)},
}};

static_assert(kTerrain.size() == kClassicTerrainDescriptorCount);

} // namespace

const ClassicTerrainDescriptor& classic_terrain_descriptor(
    std::uint16_t descriptor_index) {
    if (descriptor_index >= kTerrain.size()) {
        throw std::out_of_range("terrain descriptor outside recovered 0..76 table");
    }
    return kTerrain[descriptor_index];
}

bool classic_landing_code_product_supported(
    std::uint16_t landing_code) noexcept {
    return supported(landing_code);
}

} // namespace sensigolf
