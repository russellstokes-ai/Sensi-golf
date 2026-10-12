#pragma once
#include <cstdint>
namespace pixtee {
// Exact original Windows v1.014 computer candidate simulation
// 0x40CB6C..0x40CBC7 PRE-UPDATE contact (excludes club-12 path and
// map/code9 decisions handled by the earlier 0x40CB00 source segment).
enum class OriginalCpuContactRoute : std::uint8_t {
    ContinueMotion=0, ReturnZeroStop=1, RequestOriginalLipRandom=2
};
struct OriginalCpuContactState {
    std::int32_t vertical_force=0; // original [EDI+8]
    std::int32_t horizontal_force=0;// original [EDI+0xC]
    std::int32_t ball_height=0;    // original [EDI+0x14]
    std::int32_t distance_to_hole=0;// original [EDI+0x18]
};
struct OriginalCpuContactResult {
    OriginalCpuContactState state{};
    OriginalCpuContactRoute route=OriginalCpuContactRoute::ContinueMotion;
};
OriginalCpuContactResult original_cpu_simulation_contact(
    OriginalCpuContactState source) noexcept;
} // namespace pixtee
