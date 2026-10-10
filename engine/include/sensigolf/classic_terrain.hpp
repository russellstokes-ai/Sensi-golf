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

constexpr std::size_t kClassicTerrainDescriptorCount = 77;

const ClassicTerrainDescriptor& classic_terrain_descriptor(
    std::uint16_t descriptor_index);

bool classic_landing_code_product_supported(
    std::uint16_t landing_code) noexcept;

} // namespace sensigolf
