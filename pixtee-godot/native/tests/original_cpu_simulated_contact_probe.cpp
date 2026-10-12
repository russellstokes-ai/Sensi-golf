#include <cstdint>
#include <iostream>
#include "original_cpu_simulated_contact.hpp"
int main(){
    std::int32_t v,h,z,d;
    while(std::cin>>v>>h>>z>>d){
        const auto r=pixtee::original_cpu_simulation_contact({v,h,z,d});
        std::cout<<static_cast<int>(r.route)<<' '
                 <<r.state.vertical_force<<' '
                 <<r.state.horizontal_force<<' '
                 <<r.state.ball_height<<' '
                 <<r.state.distance_to_hole<<'\n';
    }
}
