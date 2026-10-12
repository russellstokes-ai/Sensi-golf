#include <cstdint>
#include <iostream>
#include <vector>
#include "sensigolf/recovered_animation_events.hpp"
using namespace sensigolf::recovered;
int main(){
    std::uint32_t opcode,delay,wait,offset,status,height,width,x,y;
    while(std::cin>>opcode>>delay>>wait>>offset>>status>>height>>width>>x>>y){
        if((opcode!=4&&opcode!=8)||offset>128)return 2;
        std::vector<std::uint8_t> bytes(offset+10,0);
        bytes[0]=static_cast<std::uint8_t>(delay);
        bytes[1]=static_cast<std::uint8_t>(delay>>8);
        bytes[6+offset]=static_cast<std::uint8_t>(opcode);
        bytes[7+offset]=0x80;
        bytes[8+offset]=static_cast<std::uint8_t>(wait);
        bytes[9+offset]=static_cast<std::uint8_t>(wait>>8);
        OriginalAnimatedEvent slot{};
        slot.status=static_cast<std::uint16_t>(status);
        slot.ticks=0;
        slot.height=static_cast<std::uint16_t>(height);
        slot.width=static_cast<std::uint16_t>(width);
        slot.x=static_cast<std::uint16_t>(x);
        slot.y=static_cast<std::uint16_t>(y);
        slot.asset_pointer=0x501000u;
        slot.frame_offset=offset;
        const auto result=original_animation_decode_supported_step(slot,bytes);
        std::cout<<static_cast<int>(result.kind)<<' '<<slot.status<<' '
                 <<slot.ticks<<' '<<slot.height<<' '<<slot.frame_offset<<' '
                 <<slot.asset_pointer<<' '<<slot.x<<' '<<slot.y<<' '
                 <<slot.width<<'\n';
    }
}
