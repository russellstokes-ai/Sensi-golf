#include <cstdint>
#include <iostream>
#include "original_cpu_simulated_motion.hpp"
int main(){
    std::int32_t x,y,h,height;
    std::uint32_t direction,green;
    while(std::cin>>x>>y>>h>>height>>direction>>green){
        const auto r=pixtee::original_cpu_simulated_motion(
            {x,y,h,height,static_cast<std::uint16_t>(direction),
             static_cast<std::uint16_t>(green)});
        std::cout<<static_cast<unsigned>(r.route)<<' '
                 <<r.state.x<<' '<<r.state.y<<' '
                 <<r.state.horizontal_force<<'\n';
    }
}
