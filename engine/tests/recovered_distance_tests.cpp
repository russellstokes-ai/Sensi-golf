#include <cassert>
#include <cstdint>

#include "sensigolf/recovered_distance.hpp"

using sensigolf::recovered::distance_to_hole;

namespace {

std::int32_t raw(std::uint16_t integer, std::uint16_t fraction = 0) {
    return static_cast<std::int32_t>(
        (static_cast<std::uint32_t>(integer) << 16u)
        | static_cast<std::uint32_t>(fraction));
}

} // namespace

int main() {
    // Exact cup coordinate is the scored-hole activation case.
    assert(distance_to_hole(raw(123), raw(77), 123, 77, false) == 0u);

    // Fractions are discarded exactly as in the original helper.
    assert(distance_to_hole(
        raw(126, 0xFFFFu), raw(81, 0xBFFDu), 123, 77, false) == 3u);

    // 3-4-5 integer displacement, scaled by the original 6/10 factor.
    assert(distance_to_hole(raw(126), raw(81), 123, 77, false) == 3u);

    // Integer root is taken before the 6/10 scale.
    assert(distance_to_hole(raw(131), raw(77), 123, 77, false) == 4u);

    // Green mode halves raw coordinates before integer extraction.
    assert(distance_to_hole(raw(246), raw(154), 123, 77, true) == 0u);
    assert(distance_to_hole(raw(252), raw(162), 123, 77, true) == 3u);

    return 0;
}
