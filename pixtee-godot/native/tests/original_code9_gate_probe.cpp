#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_interactions.hpp"
int main(){
    int tile,club,vertical,height;
    while(std::cin>>tile>>club>>vertical>>height){
        const auto event=sensigolf::recovered::original_code9_contact(
            static_cast<std::uint16_t>(tile),
            static_cast<std::uint16_t>(club),
            static_cast<std::int32_t>(vertical),
            static_cast<std::int32_t>(height));
        std::cout<<static_cast<int>(event)<<"\n";
    }
}
