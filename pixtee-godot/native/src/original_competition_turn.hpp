#pragma once
#include <array>
#include <cstdint>
namespace pixtee {
// Original v1.014 competition mode 4 / 6 turn-owner prefix
// 0x40C098..0x40C16D, before copying ball position / player resource.
// Source labels are raw memory fields, NOT a presumed matchplay ruleset.
struct OriginalCompetitionTurnInput {
    std::uint8_t mode=4;
    std::uint16_t active_side=0; // [0x4255de]
    std::array<std::uint32_t,2> ball_distance{}; // [0x4287d6/0x428802]+18
    std::uint16_t side0_flag=0; // [0x4255b2]
    std::uint16_t side1_flag=0; // [0x4255b4]
    std::array<std::uint8_t,4> slot_bytes{{0,1,2,3}}; // [0x4255cc..]
    std::array<std::uint16_t,2> previous_slot{}; // [0x4255da/dc]
};
struct OriginalCompetitionTurnSelection {
    int selected_slot=-1; // x86 EBX at 0x40C16D
    int source_slot=-1;   // x86 EBP
    int ball_array=-1;    // x86 ESI / 0x2C original stride
    std::uint16_t active_side=0;
    std::array<std::uint16_t,2> previous_slot{};
};
OriginalCompetitionTurnSelection original_competition_turn_select(
    const OriginalCompetitionTurnInput& state);
}
