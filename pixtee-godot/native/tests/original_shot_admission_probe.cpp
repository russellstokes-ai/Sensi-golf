#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_shot_admission.hpp"
int main() {
    std::uint32_t flags,mode5a,mode5c;
    while(std::cin >> flags >> mode5a >> mode5c) {
        auto result=sensigolf::recovered::original_shot_tick_admission({
            static_cast<std::uint16_t>(flags),
            static_cast<std::uint16_t>(mode5a),
            static_cast<std::uint16_t>(mode5c)});
        std::cout << static_cast<int>(result.route) << ' '
            << result.state.player_flags << ' '
            << result.state.player_mode_5a << ' '
            << result.state.player_mode_5c << ' '
            << (result.cpu_delegate_requested?1:0) << '\n';
    }
}
