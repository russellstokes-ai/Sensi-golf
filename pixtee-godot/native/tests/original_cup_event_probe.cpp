#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_world_boundary.hpp"
int main() {
    std::uint32_t index;
    while(std::cin>>index) {
        const auto r=sensigolf::recovered::original_cup_capture_event();
        std::cout<<static_cast<unsigned>(r.byte_420fef)<<' '
                 <<r.dword_420544<<' '<<r.dword_420548<<' '
                 <<r.source_esi<<'\n';
    }
}
