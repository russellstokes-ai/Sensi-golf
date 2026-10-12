#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_animation_events.hpp"
int main(){
    using namespace sensigolf::recovered;
    OriginalAnimationSlots slots{};
    while(true) {
        for(auto& s:slots) {
            std::uint32_t v[8]{};
            for(auto& x:v)if(!(std::cin>>x))return 0;
            s.status=static_cast<std::uint16_t>(v[0]);
            s.x=static_cast<std::uint16_t>(v[1]);
            s.y=static_cast<std::uint16_t>(v[2]);
            s.height=static_cast<std::uint16_t>(v[3]);
            s.width=static_cast<std::uint16_t>(v[4]);
            s.ticks=static_cast<std::uint16_t>(v[5]);
            s.asset_pointer=v[6];
            s.frame_offset=v[7];
        }
        auto t=original_animation_tick_countdowns(slots);
        std::cout<<t.active_slots<<' '<<t.newly_ready_for_script;
        for(const auto& s:slots)
            std::cout<<' '<<s.status<<' '<<s.x<<' '<<s.y<<' '
                     <<s.height<<' '<<s.width<<' '<<s.ticks<<' '
                     <<s.asset_pointer<<' '<<s.frame_offset;
        std::cout<<'\n';
    }
}
