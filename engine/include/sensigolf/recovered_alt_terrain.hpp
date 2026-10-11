#pragma once
#include <cstdint>
namespace sensigolf::recovered {

// Original Windows v1.014 SECOND course-flight routine 0x40CC2D..0x40CC74,
// reached in 0x40CB00 update. It is DISTINCT from original 0x40A686
// per-shot collision. Do not wire it into the active shot until its
// caller and complete dispatcher are proven.
enum class OriginalAlternateTerrainRoute : std::uint8_t {
    ReturnZero = 0,      // x86 0x40CC74; 35/60 or stopped rebound
    ReturnMinusOne = 1,  // x86 0x40CC4D; 8,9,10
    ContinueGround = 2,  // x86 0x40CBCD; positive rebound momentum
};
struct OriginalAlternateTerrainResult {
    OriginalAlternateTerrainRoute route =
        OriginalAlternateTerrainRoute::ReturnZero;
    std::int32_t vertical_force = 0;
    std::int32_t horizontal_force = 0;
};
// Original unsigned BX comparisons permit 0xFFFF to reach rebound.
OriginalAlternateTerrainResult original_alternate_terrain_contact(
    std::uint16_t landing_code, std::int32_t vertical_force,
    std::int32_t horizontal_force) noexcept;
} // namespace sensigolf::recovered
