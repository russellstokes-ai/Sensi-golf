#include "original_cpu_shot_variation.hpp"
#include <iostream>
int main(){
    int flag,power,var,a,b,c;
    while(std::cin>>flag>>power>>var>>a>>b>>c){
        const auto s=pixtee::original_cpu_shot_variation(
            flag!=0,static_cast<std::uint16_t>(power),
            static_cast<std::uint16_t>(var),
            static_cast<std::uint16_t>(a),
            static_cast<std::uint16_t>(b),
            static_cast<std::uint16_t>(c));
        std::cout<<s.captured_power<<" "<<s.difficulty_variation<<" "
                 <<s.swing_adjuster<<" "<<unsigned(s.random_calls)<<"\n";
    }
}
