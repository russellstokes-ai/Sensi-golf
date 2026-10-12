#include "original_cpu_simulated_contact.hpp"
#include <cstdint>
namespace pixtee {
namespace {
std::int32_t signed32(std::uint32_t n) noexcept {
    return n<=0x7fffffffu ? static_cast<std::int32_t>(n)
         : static_cast<std::int32_t>(
             static_cast<std::int64_t>(n)-0x100000000LL);
}
}
OriginalCpuContactResult original_cpu_simulation_contact(
    OriginalCpuContactState state) noexcept {
    // Original x86 0x40CB6C SUB 0x2100, then ADD to height. Both
    // operations wrap modulo 2^32 and the height clamp uses signed JNS.
    state.vertical_force=signed32(
        static_cast<std::uint32_t>(state.vertical_force)-0x2100u);
    state.ball_height=signed32(
        static_cast<std::uint32_t>(state.ball_height)+
        static_cast<std::uint32_t>(state.vertical_force));
    if(state.ball_height<0)state.ball_height=0;
    if(state.distance_to_hole!=0 || state.ball_height>0x32)
        return {state,OriginalCpuContactRoute::ContinueMotion};
    // The zero-distance source side-condition compares signed DWORD.
    if(state.horizontal_force<0xd000)
        return {state,OriginalCpuContactRoute::ReturnZeroStop};
    // Original 0x40CB9C..0x40CBA7: MOV V,H; SHR EAX,1;
    // H = logical(right1 of unsigned original H); then call original
    // PRNG 0x406FFA with AX=0x800. No fake sampled RNG is used.
    const auto h=static_cast<std::uint32_t>(state.horizontal_force);
    state.vertical_force=state.horizontal_force;
    state.horizontal_force=signed32(h>>1);
    return {state,OriginalCpuContactRoute::RequestOriginalLipRandom};
}
} // namespace pixtee
