#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace sensigolf {

inline constexpr std::size_t kClassicRoundHoleCount = 18;
inline constexpr std::size_t kClassicParTableSize = 100;

struct ClassicHoleResourceNames {
    std::string mapm_map;
    std::string maps_map;
    std::string mapm_spt;
};

struct ClassicHolePlanEntry {
    std::uint16_t round_index = 0;
    std::uint8_t resource_id = 0;
    std::uint8_t par = 0;
    ClassicHoleResourceNames resources{};
};

// Platform-neutral representation of the original v1.014 current-order table.
// The original executable supplies one of several 18-byte order tables. The
// commercial table bytes are imported from the user's licensed assets rather
// than duplicated in the portable source tree.
class ClassicHolePlan {
public:
    ClassicHolePlan(
        std::array<std::uint8_t, kClassicRoundHoleCount> order,
        std::array<std::uint8_t, kClassicParTableSize> par_by_resource_id);

    ClassicHolePlanEntry hole(std::size_t round_index) const;
    const std::array<std::uint8_t, kClassicRoundHoleCount>& order() const noexcept;

    static ClassicHoleResourceNames resource_names(
        std::uint8_t resource_id);

private:
    std::array<std::uint8_t, kClassicRoundHoleCount> order_{};
    std::array<std::uint8_t, kClassicParTableSize> par_by_resource_id_{};
};

} // namespace sensigolf
