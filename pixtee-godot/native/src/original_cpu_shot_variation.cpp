#include "original_cpu_shot_variation.hpp"
namespace pixtee {
OriginalCpuShotVariation original_cpu_shot_variation(
    bool bypass, std::uint16_t power, std::uint16_t variation,
    std::uint16_t random100, std::uint16_t random_power,
    std::uint16_t random_direction) noexcept {
    OriginalCpuShotVariation state{power,variation,0,0};
    // 0x40C7DD: CPU special bit bypasses stochastic adjustment.
    if(bypass) return state;
    state.random_calls=1;
    // 0x40C7E4..0x40C7F4: no error in the source when AX <= 15.
    if(random100<=15) return state;
    if(random100>80)
        state.difficulty_variation=static_cast<std::uint16_t>(
            state.difficulty_variation+(random100>>5u));
    state.random_calls=2;
    // 0x40C80F..0x40C833. Native x86 WORD sub and its SF matter.
    const auto delta=static_cast<std::uint16_t>(random_power-6u);
    const std::uint16_t adjusted=static_cast<std::uint16_t>(
        state.captured_power-delta);
    state.captured_power=static_cast<std::int16_t>(adjusted)<0
        ? static_cast<std::uint16_t>(5):adjusted;
    if(state.difficulty_variation==0) return state;
    state.random_calls=3;
    // 0x40C833..0x40C84A source two's complement sign-extended word.
    const auto curve=static_cast<std::uint16_t>(
        random_direction-state.difficulty_variation);
    state.swing_adjuster=static_cast<std::int16_t>(curve);
    return state;
}
}
