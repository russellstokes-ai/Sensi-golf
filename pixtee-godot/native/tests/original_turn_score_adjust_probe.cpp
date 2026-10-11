#include "original_turn_score_adjust.hpp"
#include <iostream>
int main(){
    unsigned n;
    while(std::cin>>n){
        pixtee::OriginalTurnScores state{};
        state.count=static_cast<std::uint8_t>(n);
        for(int i=0;i<4;++i){
            unsigned stroke,total,marked,distance;
            std::cin>>stroke>>total>>marked>>distance;
            state.strokes[i]=static_cast<std::uint16_t>(stroke);
            state.total_strokes[i]=static_cast<std::uint16_t>(total);
            state.completed_marker[i]=static_cast<std::uint16_t>(marked);
            state.ball_distance[i]=distance;
        }
        try {
            const auto v=pixtee::original_turn_score_adjust(state);
            std::cout<<v.corrected_index;
            for(int i=0;i<4;++i)
                std::cout<<" "<<v.result.strokes[i]<<" "<<v.result.total_strokes[i];
            std::cout<<"\n";
        } catch(const std::exception&){std::cout<<"ERR\n";}
    }
}
