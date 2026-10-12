#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_animation_events.hpp"
using namespace sensigolf::recovered;
int main() {
    for(;;) {
        OriginalAnimationSlots slots{};
        for(std::size_t i=0;i<slots.size();++i) {
            unsigned long long n[8];
            for(unsigned j=0;j<8;++j) {
                if(!(std::cin>>n[j])) {
                    if(i==0 && j==0)return 0;
                    return 2;
                }
                if(n[j]>(j<6?0xffffULL:0xffffffffULL))return 3;
            }
            auto& s=slots[i];
            s.status=static_cast<std::uint16_t>(n[0]);
            s.x=static_cast<std::uint16_t>(n[1]);
            s.y=static_cast<std::uint16_t>(n[2]);
            s.height=static_cast<std::uint16_t>(n[3]);
            s.width=static_cast<std::uint16_t>(n[4]);
            s.ticks=static_cast<std::uint16_t>(n[5]);
            s.asset_pointer=static_cast<std::uint32_t>(n[6]);
            s.frame_offset=static_cast<std::uint32_t>(n[7]);
        }
        original_animation_reset_all(slots);
        for(const auto& s:slots) {
            std::cout<<s.status<<' '<<s.x<<' '<<s.y<<' '
                     <<s.height<<' '<<s.width<<' '<<s.ticks<<' '
                     <<s.asset_pointer<<' '<<s.frame_offset<<' ';
        }
        std::cout<<'\n';
    }
}
