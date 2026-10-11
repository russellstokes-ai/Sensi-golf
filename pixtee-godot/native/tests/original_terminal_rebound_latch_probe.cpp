#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_interactions.hpp"

int main() {
    std::uint32_t flags;
    std::int32_t vertical, horizontal;
    while(std::cin >> flags >> vertical >> horizontal) {
        const auto out=sensigolf::recovered::original_terminal_rebound_with_latch(
            static_cast<std::uint16_t>(flags),vertical,horizontal);
        std::cout << out.rebound.vertical_force << ' '
                  << out.rebound.horizontal_force << ' '
                  << (out.rebound.continues_flight ? 1 : 0) << ' '
                  << out.player_flags << ' '
                  << (out.event_zero_requested ? 1 : 0) << '\n';
    }
}
