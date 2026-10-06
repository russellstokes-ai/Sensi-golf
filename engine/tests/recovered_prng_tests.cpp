#include <cassert>
#include "sensigolf/recovered_prng.hpp"

using sensigolf::recovered::OriginalPrng16;

int main() {
    OriginalPrng16 rng{0x1234, 0xABCD};

    assert(rng.next(0x0200) == 0x0075);
    assert(rng.seed0 == 0x3A6D);
    assert(rng.seed1 == 0xD5E6);

    assert(rng.next(0x0800) == 0x0034);
    assert(rng.seed0 == 0x068F);
    assert(rng.seed1 == 0x6AF3);

    assert(rng.next(0x0090) == 0x001D);
    assert(rng.seed0 == 0x3478);
    assert(rng.seed1 == 0x6AF3);

    assert(rng.next(0x0011) == 0x000E);
    assert(rng.seed0 == 0xC932);
    assert(rng.seed1 == 0xB579);

    assert(rng.next(0xFFFF) == 0x4996);
    assert(rng.seed0 == 0x4996);
    assert(rng.seed1 == 0xB579);

    return 0;
}
