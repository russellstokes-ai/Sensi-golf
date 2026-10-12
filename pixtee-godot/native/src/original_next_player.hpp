#pragma once
#include <array>
#include <cstdint>

namespace pixtee {
// Original Windows v1.014 "pick next player" decision at 0x40C1B7..0x40C22F.
// Decision-only, not yet entire turn lifecycle or game-mode controller.
struct OriginalPlayerChoiceInput {
    std::uint8_t count = 1;
    std::uint8_t forced_holder = 0; // 0x41F885, zero = metric path
    std::array<std::uint32_t,4> priority_metric{}; // 0x4287D6+0x18 + 0x2C*i
    std::array<std::uint16_t,4> eliminated{}; // 0x425FE2+0x5E + 0x86*i
    std::array<std::uint8_t,4> identity{{0,1,2,3}}; // +0x6E
};
std::uint8_t original_next_player_index(
    const OriginalPlayerChoiceInput& input);
}
