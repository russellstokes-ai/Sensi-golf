#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_alt_terrain.hpp"
int main() {
    std::uint32_t code;
    std::int32_t v,h;
    while(std::cin>>code>>v>>h) {
        const auto r=sensigolf::recovered::original_alternate_terrain_contact(
            static_cast<std::uint16_t>(code),v,h);
        std::cout<<static_cast<int>(r.route)<<' '
                 <<r.vertical_force<<' '
                 <<r.horizontal_force<<'\n';
    }
}
