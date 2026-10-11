#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_interactions.hpp"
int main(){
    int x,y,height,flagx,flagy,green;
    while(std::cin>>x>>y>>height>>flagx>>flagy>>green){
        sensigolf::recovered::FlightState b{};
        b.x=static_cast<std::int32_t>(x)<<16;
        b.y=static_cast<std::int32_t>(y)<<16;
        b.height=static_cast<std::int32_t>(height);
        std::cout<<(sensigolf::recovered::original_flag_coordinate_hit(
            b,static_cast<std::int16_t>(flagx),
            static_cast<std::int16_t>(flagy),green!=0)?1:0)<<"\n";
    }
}
