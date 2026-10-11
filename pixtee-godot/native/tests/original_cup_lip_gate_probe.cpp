#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_interactions.hpp"
int main(){
    int distance_zero,player_mode,height,horizontal;
    while(std::cin>>distance_zero>>player_mode>>height>>horizontal){
        sensigolf::recovered::FlightState ball{};
        ball.height=static_cast<std::int32_t>(height);
        ball.horizontal_force=static_cast<std::int32_t>(horizontal);
        const bool activated=sensigolf::recovered::original_near_cup_lip_contact(
            distance_zero!=0,static_cast<std::uint16_t>(player_mode),ball);
        std::cout<<(activated?1:0)<<"\n";
    }
}
