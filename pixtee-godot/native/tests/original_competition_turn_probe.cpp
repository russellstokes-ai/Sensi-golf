#include "original_competition_turn.hpp"
#include <iostream>
int main(){
    unsigned mode,side,d0,d1,f0,f1,p0,p1,a,b,c,d;
    while(std::cin>>mode>>side>>d0>>d1>>f0>>f1>>p0>>p1>>a>>b>>c>>d) {
        pixtee::OriginalCompetitionTurnInput in{};
        in.mode=static_cast<std::uint8_t>(mode);
        in.active_side=static_cast<std::uint16_t>(side);
        in.ball_distance={d0,d1};
        in.side0_flag=static_cast<std::uint16_t>(f0);
        in.side1_flag=static_cast<std::uint16_t>(f1);
        in.previous_slot={static_cast<std::uint16_t>(p0),static_cast<std::uint16_t>(p1)};
        in.slot_bytes={static_cast<std::uint8_t>(a),static_cast<std::uint8_t>(b),
                       static_cast<std::uint8_t>(c),static_cast<std::uint8_t>(d)};
        try {
            const auto o=pixtee::original_competition_turn_select(in);
            std::cout<<o.selected_slot<<" "<<o.source_slot<<" "<<o.ball_array<<" "
                <<o.active_side<<" "<<o.previous_slot[0]<<" "<<o.previous_slot[1]<<"\n";
        } catch(const std::exception&) {std::cout<<"ERR\n";}
    }
}
