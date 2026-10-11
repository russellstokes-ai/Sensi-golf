#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace sensigolf {

struct ClassicTerrainDescriptor {
    std::uint16_t landing_code = 0;
    std::int16_t variant = 0;
    std::uint16_t profile_slot = 0;
    std::string_view name{};
    bool product_supported = false;
};

// Identifies the ORIGINAL descriptor label without inventing different
// physics: numeric landing code 35 is shared by several named surfaces.
enum class ClassicNamedHazard : std::uint8_t {
    None = 0,
    Water = 1,
    NoGo = 2,
    OutOfBoundsTile = 3,
    OtherCode35 = 4,
    OutsideWorld = 5, // source lookup negative AX; never a MAPI descriptor
};

ClassicNamedHazard classic_named_hazard(
    std::uint16_t descriptor_index);

constexpr std::size_t kClassicTerrainDescriptorCount = 77;

const ClassicTerrainDescriptor& classic_terrain_descriptor(
    std::uint16_t descriptor_index);

bool classic_landing_code_product_supported(
    std::uint16_t landing_code) noexcept;

} // namespace sensigolf
