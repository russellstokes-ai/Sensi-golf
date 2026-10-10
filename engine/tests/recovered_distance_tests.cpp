#include <cassert>
#include <cstdint>

#include "sensigolf/recovered_distance.hpp"

using sensigolf::recovered::distance_to_hole;

int main() {
    // Exact cup coordinate.
    assert(distance_to_hole(
        123 << 16, 77 << 16, 123, 77, false) == 0);

    // Integer 3-4-5 distance, then original 6/10 scaling.
    assert(distance_to_hole(
        126 << 16, 81 << 16, 123, 77, false) == 3);

    // Original scale floors short distances to zero.
    assert(distance_to_hole(
        124 << 16, 78 << 16, 123, 77, false) == 0);

    // Fractional 16.16 components do not alter integer extraction.
    assert(distance_to_hole(
        (126 << 16) | 0xFFFF,
        (81 << 16) | 0xBEEF,
        123, 77, false) == 3);

    // Green mode arithmetic-halves the raw coordinate space first.
    assert(distance_to_hole(
        (246 + 6) << 16,
        (154 + 8) << 16,
        123, 77, true) == 3);

    return 0;
}
