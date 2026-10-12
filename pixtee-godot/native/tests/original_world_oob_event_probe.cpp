#include <iostream>
#include "sensigolf/recovered_world_boundary.hpp"
int main() {
    std::uint32_t ignored;
    while (std::cin >> ignored) {
        const auto r=sensigolf::recovered::original_world_oob_event();
        std::cout << static_cast<unsigned>(r.byte_420fef) << ' '
            << r.dword_420544 << ' ' << r.dword_420548 << ' '
            << r.source_esi << '\n';
    }
}
