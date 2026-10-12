#include "original_cpu_difficulty.hpp"
#include <stdexcept>
namespace pixtee {
OriginalComputerSearchParameters original_cpu_search_parameters(
    std::uint16_t mode, bool special) {
    if(mode<4 || mode>15)
        throw std::invalid_argument("CPU mode must be verified original index 4..15");
    const auto delta=static_cast<std::uint16_t>(mode-4u);
    const auto stride=static_cast<std::uint16_t>(delta+(delta>>1u));
    const auto variation=static_cast<std::uint16_t>(delta>>2u);
    const auto budget=special ? std::uint16_t{0x6e} :
        static_cast<std::uint16_t>(0x6du-stride);
    return {stride,variation,budget};
}
}
