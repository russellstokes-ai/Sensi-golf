#pragma once
#include <cstdint>
namespace pixtee {
struct OriginalComputerSearchParameters {
    std::uint16_t candidate_stride = 0;
    std::uint16_t variation_budget = 0;
    std::uint16_t search_budget = 0;
};
// Machine source 0x40C665..0x40C6B2: only the original difficulty
// parameters. NOT club choice, aim, opponent playing strategy or random seed.
OriginalComputerSearchParameters original_cpu_search_parameters(
    std::uint16_t player_mode, bool special_flag);
}
