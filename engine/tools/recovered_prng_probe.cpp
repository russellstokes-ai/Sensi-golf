#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

#include "sensigolf/recovered_prng.hpp"

using sensigolf::recovered::OriginalPrng16;

static unsigned long parse(const char* s) {
    char* end = nullptr;
    const auto v = std::strtoul(s, &end, 0);
    if (!end || *end != '\0' || v > 0xFFFFul) {
        throw std::invalid_argument("expected 16-bit integer");
    }
    return v;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "usage: recovered_prng_probe <seed0> <seed1> <max> [max ...]\n";
        return 2;
    }
    try {
        OriginalPrng16 rng{
            static_cast<std::uint16_t>(parse(argv[1])),
            static_cast<std::uint16_t>(parse(argv[2]))};

        std::cout << "{\n  \"samples\":[\n";
        for (int i = 3; i < argc; ++i) {
            const auto maxv = static_cast<std::uint16_t>(parse(argv[i]));
            const auto value = rng.next(maxv);
            if (i != 3) std::cout << ",\n";
            std::cout << "    {\"max\":" << maxv
                      << ",\"value\":" << value
                      << ",\"seed0\":" << rng.seed0
                      << ",\"seed1\":" << rng.seed1 << "}";
        }
        std::cout << "\n  ]\n}\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
