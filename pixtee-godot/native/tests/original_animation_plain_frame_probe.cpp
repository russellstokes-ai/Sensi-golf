#include <cstdint>
#include <iostream>
#include <vector>
#include "sensigolf/recovered_animation_events.hpp"
int main(){
    std::uint32_t delay,frame,offset,height,width;
    while(std::cin>>delay>>frame>>offset>>height>>width) {
        if(offset>2048 || frame>0x7fff || delay>65535)return 2;
        std::vector<std::uint8_t> bytes(offset+8u,0);
        bytes[0]=static_cast<std::uint8_t>(delay);
        bytes[1]=static_cast<std::uint8_t>(delay>>8);
        bytes[offset+6u]=static_cast<std::uint8_t>(frame);
        bytes[offset+7u]=static_cast<std::uint8_t>(frame>>8);
        sensigolf::recovered::OriginalAnimatedEvent slot{};
        slot.status=0;
        slot.ticks=0;
        slot.height=static_cast<std::uint16_t>(height);
        slot.width=static_cast<std::uint16_t>(width);
        slot.frame_offset=offset;
        const auto result=sensigolf::recovered::original_animation_decode_plain_frame(
            slot,bytes);
        std::cout<<result.displayed_frame<<' '<<result.delay<<' '
                 <<slot.height<<' '<<slot.ticks<<' '<<slot.frame_offset
                 <<' '<<slot.width<<'\n';
    }
}
