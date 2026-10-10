#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "sensigolf/classic_hole_plan.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: sensigolf_classic_hole_plan_probe <resource-id>\n";
        return 2;
    }

    try {
        const auto raw = std::stoul(argv[1], nullptr, 0);
        if (raw > 255u) {
            throw std::out_of_range("resource id");
        }
        const auto names = sensigolf::ClassicHolePlan::resource_names(
            static_cast<std::uint8_t>(raw));
        std::cout
            << "{\"resource_id\":" << raw
            << ",\"mapm_map\":\"" << names.mapm_map
            << "\",\"maps_map\":\"" << names.maps_map
            << "\",\"mapm_spt\":\"" << names.mapm_spt
            << "\"}\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
