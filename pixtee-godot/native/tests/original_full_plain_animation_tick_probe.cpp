#include <array>
#include <cstdint>
#include <iostream>
#include <vector>
#include "sensigolf/recovered_animation_events.hpp"
using namespace sensigolf::recovered;
int main(){
    while(true){
        OriginalAnimationSlots slots{};
        std::vector<OriginalAnimationScriptAsset> scripts;
        scripts.reserve(10);
        std::uint32_t value;
        for(std::size_t i=0;i<10;++i){
            std::uint32_t status,ticks,offset,frame,delay,height,width;
            if(!(std::cin>>status>>ticks>>offset>>frame>>delay>>height>>width)){
                if(i==0)return 0;
                return 2;
            }
            if(offset>64 || frame>0x7fff)return 2;
            auto& s=slots[i];
            s.status=static_cast<std::uint16_t>(status);
            s.ticks=static_cast<std::uint16_t>(ticks);
            s.frame_offset=offset;
            s.height=static_cast<std::uint16_t>(height);
            s.width=static_cast<std::uint16_t>(width);
            s.asset_pointer=static_cast<std::uint32_t>(0x700000+i*128);
            std::vector<std::uint8_t> b(offset+8,0);
            b[0]=static_cast<std::uint8_t>(delay);
            b[1]=static_cast<std::uint8_t>(delay>>8);
            b[6+offset]=static_cast<std::uint8_t>(frame);
            b[7+offset]=static_cast<std::uint8_t>(frame>>8);
            scripts.push_back({s.asset_pointer,std::move(b)});
        }
        (void)original_animation_tick_with_plain_scripts(slots,scripts);
        for(const auto& s:slots)
            std::cout<<s.status<<' '<<s.ticks<<' '
                     <<s.height<<' '<<s.frame_offset<<' '
                     <<s.width<<' ';
        std::cout<<'\n';
    }
}
