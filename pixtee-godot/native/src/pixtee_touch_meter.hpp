#pragma once
#include <algorithm>
#include <cstdint>
namespace pixtee {
// Owner-specified mobile input timing. Shot outcomes still use recovered controls.
// This is not a claim of original executable meter/display parity.
struct TouchMeter {
    int stage=0,position=160,power=0,lower=55,upper=70,red_half=48,yellow=64,ticks=0;
    void start(int low,int high) noexcept {
        stage=1;position=160;power=0;ticks=0;lower=low;upper=high;
        const int span=std::clamp(high-low+1,1,16);
        red_half=std::max(8,span*3);yellow=std::max(12,span*4);
    }
    int sample() const noexcept {
        const int delta=160-position;
        if(std::abs(delta)<=red_half)return 63;
        if(delta>0)return std::clamp(64+(delta-red_half-1)*std::max(1,upper-63)/yellow,64,105);
        return std::clamp(62-(-delta-red_half-1)*std::max(1,63-lower)/yellow,0,62);
    }
    bool step(bool pressed) noexcept {
        if(stage==1 && pressed) {power=std::clamp((position-160)/10,0,105);stage=2;return false;}
        if(stage==2 && pressed) {stage=0;return true;}
        if(stage==0)return false;
        // Mobile V2: fixed ticks, 2.50 s full charge, no skipped ticks.
        ++ticks;
        if(stage==1) {
            position=std::min(1210,position+6);
            if(position==1210){power=105;stage=2;}
        }else {position=std::max(0,position-7);if(position==0)stage=0;}
        return false;
    }
};
}
