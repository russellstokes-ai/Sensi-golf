#pragma once
#include <cstdint>
namespace pixtee {
// Original v1.014 SECOND ball/candidate motion tick 0x40CBCD..0x40CC9F.
// This is not the player's main 0x40A2.. shot tick. Only accepts an
// authentic caller path that reached 0x40CBCD, after source CPU contact
// and map/club checks; does not guess original scene object geometry.
enum class OriginalCpuMotionRoute : std::uint8_t {
    NextOriginalTrialTick=0,      // source 0x40CC9F -> 0x40CB00
    RequiresMappedContact=1,     // source 0x40CC05 before x/y movement
};
struct OriginalCpuMotionState {
    std::int32_t x=0,y=0;
    std::int32_t horizontal_force=0;
    std::int32_t height=0;
    std::uint16_t direction=0;
    std::uint16_t green_mode=0;
};
struct OriginalCpuMotionResult {
    OriginalCpuMotionState state{};
    OriginalCpuMotionRoute route=OriginalCpuMotionRoute::NextOriginalTrialTick;
};
OriginalCpuMotionResult original_cpu_simulated_motion(
    OriginalCpuMotionState in) noexcept;
} // namespace pixtee
