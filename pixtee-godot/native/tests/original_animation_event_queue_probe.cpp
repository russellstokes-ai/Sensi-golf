#include <array>
#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_animation_events.hpp"
int main(){
    using namespace sensigolf::recovered;
    std::uint32_t x,y,w,h,index;
    while(std::cin>>x>>y>>w>>h>>index){
        OriginalAnimationAssetTable assets{};
        for(auto& v:assets)std::cin>>v;
        OriginalAnimationSlots slots{};
        for(auto& slot:slots){
            std::uint32_t v[8]{};
            for(auto& q:v)std::cin>>q;
            slot.status=static_cast<std::uint16_t>(v[0]);
            slot.x=static_cast<std::uint16_t>(v[1]);
            slot.y=static_cast<std::uint16_t>(v[2]);
            slot.height=static_cast<std::uint16_t>(v[3]);
            slot.width=static_cast<std::uint16_t>(v[4]);
            slot.ticks=static_cast<std::uint16_t>(v[5]);
            slot.asset_pointer=v[6];
            slot.frame_offset=v[7];
        }
        const auto r=original_animation_enqueue(
            slots,
            {static_cast<std::uint16_t>(x),static_cast<std::uint16_t>(y),
             static_cast<std::uint16_t>(w),static_cast<std::uint16_t>(h),
             static_cast<std::uint16_t>(index)},assets);
        std::cout<<r.slot;
        for(const auto& a:slots)
            std::cout<<' '<<a.status<<' '<<a.x<<' '<<a.y<<' '
                     <<a.height<<' '<<a.width<<' '<<a.ticks<<' '
                     <<a.asset_pointer<<' '<<a.frame_offset;
        std::cout<<'\n';
    }
}
