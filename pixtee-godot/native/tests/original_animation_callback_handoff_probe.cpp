#include <cstdint>
#include <iostream>
#include <vector>
#include "sensigolf/recovered_animation_events.hpp"
using namespace sensigolf::recovered;
int main() {
    std::uint32_t delay,offset,status,x,y,height,width;
    while(std::cin>>delay>>offset>>status>>x>>y>>height>>width) {
        if(delay>65535 || offset>128 || status>=65535 ||
           x>65535 || y>65535 || height>65535 || width>65535)
            return 2;
        OriginalAnimatedEvent slot{};
        slot.status=static_cast<std::uint16_t>(status);
        slot.ticks=0;
        slot.x=static_cast<std::uint16_t>(x);
        slot.y=static_cast<std::uint16_t>(y);
        slot.height=static_cast<std::uint16_t>(height);
        slot.width=static_cast<std::uint16_t>(width);
        slot.asset_pointer=0x501000u;
        slot.frame_offset=offset;
        std::vector<std::uint8_t> script(offset+12u,0);
        script[0]=static_cast<std::uint8_t>(delay);
        script[1]=static_cast<std::uint8_t>(delay>>8);
        script[offset+6u]=0;
        script[offset+7u]=0x80;
        // Test-only callback RET at 0x500200, never called in native.
        script[offset+8u]=0;
        script[offset+9u]=2;
        script[offset+10u]=0x50;
        script[offset+11u]=0;
        const auto handoff=original_animation_prepare_callback(slot,script);
        if(handoff.source_callback_token!=0x500200u)return 3;
        if(slot.ticks!=0 || slot.frame_offset!=offset)return 4;
        // This is a simulated host acknowledgement, NOT executing
        // a callback pointer from user-provided bytes.
        original_animation_finalize_callback(slot,handoff);
        std::cout<<slot.status<<' '<<slot.ticks<<' '<<slot.height<<' '
                 <<slot.frame_offset<<' '<<slot.asset_pointer<<' '
                 <<slot.x<<' '<<slot.y<<' '<<slot.width<<' '
                 <<handoff.source_callback_token<<'\n';
    }
    return 0;
}
