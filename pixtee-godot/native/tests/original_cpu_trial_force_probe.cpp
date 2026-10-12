#include <cstdint>
#include <iostream>
#include "original_cpu_trial_force.hpp"
int main() {
    std::int32_t scale,v,h;
    std::uint32_t power;
    while(std::cin>>scale>>v>>h>>power) {
        const auto out=pixtee::original_cpu_trial_force(
          {scale,v,h,static_cast<std::uint16_t>(power)});
        std::cout<<out.vertical_force<<' '<<out.horizontal_force<<'\n';
    }
}
