#include <cstdint>
#include <iostream>
#include "original_cpu_quality_score.hpp"
int main() {
    std::uint32_t distance,code,green;
    const auto& penalties=pixtee::original_cpu_reference_penalties();
    while(std::cin>>distance>>code>>green) {
        const auto value=pixtee::original_cpu_quality_score(
            static_cast<std::uint16_t>(distance),
            static_cast<std::uint16_t>(code),
            static_cast<std::uint16_t>(green),penalties);
        std::cout<<value<<'\n';
    }
}
