#include <cstdint>
#include <iostream>
#include "original_cpu_trial_preflight.hpp"
int main() {
    std::uint32_t terrain,club;
    std::int32_t v,h,height;
    while(std::cin>>terrain>>club>>v>>h>>height) {
        const auto route=pixtee::original_cpu_trial_preflight({
            static_cast<std::uint16_t>(terrain),
            static_cast<std::uint16_t>(club),v,h,height});
        std::cout<<static_cast<int>(route)<<'\n';
    }
}
