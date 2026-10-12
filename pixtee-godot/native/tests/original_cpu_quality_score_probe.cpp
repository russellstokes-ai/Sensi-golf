#include <array>
#include <cstdint>
#include <iostream>
#include "original_cpu_quality_score.hpp"
int main(){
    std::uint32_t distance,code,green;
    while(std::cin>>distance>>code>>green){
        pixtee::OriginalCpuPenaltyTable table{};
        for(auto& value:table){
            std::uint32_t v=0;
            if(!(std::cin>>v))return 1;
            value=static_cast<std::uint16_t>(v);
        }
        const auto result=pixtee::original_cpu_quality_score(
          static_cast<std::uint16_t>(distance),
          static_cast<std::uint16_t>(code),
          static_cast<std::uint16_t>(green),table);
        std::cout<<result<<'\n';
    }
}
