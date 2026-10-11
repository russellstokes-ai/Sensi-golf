#include <cstdint>
#include <iostream>
#include "original_next_player.hpp"
int main(){
    unsigned count,forced;
    while(std::cin>>count>>forced){
        pixtee::OriginalPlayerChoiceInput in{};
        in.count=static_cast<std::uint8_t>(count);
        in.forced_holder=static_cast<std::uint8_t>(forced);
        for(int i=0;i<4;++i){
            unsigned id,eliminated,metric;
            std::cin>>id>>eliminated>>metric;
            in.identity[i]=static_cast<std::uint8_t>(id);
            in.eliminated[i]=static_cast<std::uint16_t>(eliminated);
            in.priority_metric[i]=metric;
        }
        try{std::cout<<unsigned(pixtee::original_next_player_index(in))<<"\n";}
        catch(const std::exception&){std::cout<<"ERR\n";}
    }
}
