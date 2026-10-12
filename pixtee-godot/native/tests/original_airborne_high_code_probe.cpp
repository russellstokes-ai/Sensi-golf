#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_interactions.hpp"
int main() {
    std::uint32_t code,club,flags;
    std::int32_t vertical,horizontal,height;
    while(std::cin >> code >> club >> flags >> vertical >> horizontal >> height) {
        const auto out=sensigolf::recovered::original_airborne_high_code_contact(
            static_cast<std::uint16_t>(code),static_cast<std::uint16_t>(club),
            static_cast<std::uint16_t>(flags),vertical,horizontal,height);
        std::cout << (out.handled?1:0) << ' '
                  << out.vertical_force << ' ' << out.horizontal_force << ' '
                  << out.ball_height << ' ' << out.player_state_flags << ' '
                  << (out.continues_flight?1:0) << ' '
                  << static_cast<unsigned>(out.event_code) << '\n';
    }
}
