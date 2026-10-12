#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_interactions.hpp"

int main() {
    std::int32_t vertical, horizontal;
    while(std::cin >> vertical >> horizontal) {
        const auto out=sensigolf::recovered::original_terminal_rebound(
            vertical,horizontal);
        std::cout << out.vertical_force << ' ' << out.horizontal_force
                  << ' ' << (out.continues_flight ? 1 : 0) << '\n';
    }
}
