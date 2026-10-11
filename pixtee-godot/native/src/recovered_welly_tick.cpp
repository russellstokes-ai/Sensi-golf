#include "recovered_welly_tick.hpp"
#include <cstdint>
namespace pixtee {
void OriginalWellyTick::reset() noexcept {state_={};}
void OriginalWellyTick::complete() noexcept {
    state_.stage=0;
    state_.step_setting=5;
    state_.angle=0;
    state_.capture_flag=0;
    state_.saved_angle=0;
}
void OriginalWellyTick::step(bool pressed,std::uint16_t club) noexcept {
    if(state_.stage==0) return;
    // Original v1.014 0x40768B..0x407782 rising Welly producer.
    if(state_.stage==1) {
        if(!state_.capture_flag && pressed) {
            state_.captured_power=state_.counter;
            state_.saved_angle=state_.angle;
            state_.capture_flag=1;
            pressed=false; // original clears byte[0x41F000] immediately
        }
        state_.counter=static_cast<std::uint16_t>(state_.counter+1u);
        auto quotient=static_cast<std::uint16_t>(state_.counter/20u);
        if(club==12 && quotient>=2) quotient=2;
        state_.step_setting=static_cast<std::uint16_t>(5u-quotient);
        state_.accumulator+=0x6000u;
        const auto angular_step=static_cast<std::uint16_t>(state_.accumulator>>16u);
        state_.angle=static_cast<std::uint16_t>(
            (state_.angle-angular_step)&0x0fffu);
        if(state_.counter>=105) {
            state_.stage=2;
            state_.counter=70;
            state_.accuracy_counter=0;
        }
        return;
    }
    // Original v1.014 0x407787..0x4078D7 descending Welly producer.
    if(state_.stage==2) {
        if(!state_.capture_flag && pressed) {
            const auto half=static_cast<std::uint16_t>(state_.counter>>1u);
            state_.counter=static_cast<std::uint16_t>(state_.counter+half);
            state_.captured_power=state_.counter;
            state_.saved_angle=state_.angle;
            state_.capture_flag=1;
            pressed=false; // captured first press consumed in original
        }
        state_.counter=static_cast<std::uint16_t>(state_.counter-1u);
        auto speed=static_cast<std::uint16_t>(state_.accuracy_counter/10u+3u);
        if(club==12) {
            if(speed<=4) speed=4;
            if(speed>=6) speed=6;
        }
        state_.step_setting=speed;
        if(state_.capture_flag && pressed) {
            complete();
            return;
        }
        state_.accuracy_counter=static_cast<std::uint16_t>(
            state_.accuracy_counter+1u);
        if(state_.accuracy_counter>70) {
            complete();
            return;
        }
        const auto angular_step=static_cast<std::uint16_t>(state_.accumulator>>16u);
        state_.angle=static_cast<std::uint16_t>(
            (state_.angle+angular_step)&0x0fffu);
    }
}
} // namespace pixtee
