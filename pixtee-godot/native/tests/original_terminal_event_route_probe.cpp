#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_interactions.hpp"

int main() {
    int landing, height;
    while (std::cin >> landing >> height) {
        const auto route=sensigolf::recovered::original_terminal_stop_contact(
            static_cast<std::uint16_t>(landing),
            static_cast<std::int16_t>(height));
        std::cout << static_cast<int>(route) << '\n';
    }
}
