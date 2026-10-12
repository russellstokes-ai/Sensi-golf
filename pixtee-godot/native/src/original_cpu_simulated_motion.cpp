#include "original_cpu_simulated_motion.hpp"
#include <cstdint>
#include "sensigolf/recovered_math.hpp"
namespace pixtee {
namespace {
std::int32_t as_i32(std::uint32_t n) noexcept {
    return n<=0x7fffffffu ? static_cast<std::int32_t>(n)
        : static_cast<std::int32_t>(
            static_cast<std::int64_t>(n)-0x100000000LL);
}
}
OriginalCpuMotionResult original_cpu_simulated_motion(
    OriginalCpuMotionState in) noexcept {
    OriginalCpuMotionResult out{};
    out.state=in;
    // Original x86 0x40CBCD begins from a caller-verified contact/
    // flight update, and may skip x/y projection after horizontal drag.
    std::int32_t remaining=in.horizontal_force;
    if(remaining<=0)remaining=0;
    else {
        const std::int32_t drag=
            in.green_mode!=0u ? 0x780 : 0xf00;
        remaining=as_i32(static_cast<std::uint32_t>(remaining)
                         -static_cast<std::uint32_t>(drag));
        if(remaining<0) {
            out.state.horizontal_force=0;
            if(in.height==0)
                out.route=OriginalCpuMotionRoute::RequiresMappedContact;
            return out;
        }
    }
    out.state.horizontal_force=remaining;
    const auto [dx,dy]=sensigolf::recovered::project_horizontal(
        remaining,in.direction);
    out.state.x=as_i32(
        static_cast<std::uint32_t>(in.x)+static_cast<std::uint32_t>(dx));
    out.state.y=as_i32(
        static_cast<std::uint32_t>(in.y)+static_cast<std::uint32_t>(dy));
    return out;
}
} // namespace pixtee
