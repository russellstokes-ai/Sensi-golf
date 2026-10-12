#include "original_cpu_trial_force.hpp"
#include <cstdint>
namespace pixtee {
namespace {
std::int32_t original_signed32(std::uint32_t value) noexcept {
    return value<=0x7fffffffu
        ? static_cast<std::int32_t>(value)
        : static_cast<std::int32_t>(static_cast<std::int64_t>(value)
                                   -0x100000000LL);
}
}
OriginalCpuTrialForce original_cpu_trial_force(
    OriginalCpuTrialForceInput input) noexcept {
    // Original x86:
    // MOV EAX,[ESI+0x32] ; MOVZX EBX,WORD[0x41D59E] ;
    // IMUL EAX,EBX (low 32 only) ; ADD EBX,[ESI+0x2A] etc.
    // All arithmetic is modulo 2^32, never C++ signed overflow.
    const auto scaled=static_cast<std::uint32_t>(input.player_shot_scale)
                       *static_cast<std::uint32_t>(input.captured_power);
    return {
        original_signed32(
            static_cast<std::uint32_t>(input.vertical_base)+scaled),
        original_signed32(
            static_cast<std::uint32_t>(input.horizontal_base)+scaled)
    };
}
} // namespace pixtee
