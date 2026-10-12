#include "original_cpu_heading_sweep.hpp"
#include <iostream>
int main(){unsigned mode,band,heading;while(std::cin>>mode>>band>>heading){
    try{
        const auto data=pixtee::original_cpu_heading_sweep_from_original_table(
            static_cast<std::uint16_t>(mode),static_cast<std::uint8_t>(band),
            static_cast<std::uint16_t>(heading));
        std::cout<<data.candidate_headings.size()<<" "<<data.final_ball_heading<<" "
                 <<data.final_player_heading<<" "<<data.final_scan_counter;
        for(const auto h:data.candidate_headings)std::cout<<" "<<h;
        std::cout<<"\n";
    }catch(const std::exception&){std::cout<<"ERR\n";}
}}
