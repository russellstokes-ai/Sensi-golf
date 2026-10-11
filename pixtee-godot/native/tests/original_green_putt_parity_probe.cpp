#include <cstdint>
#include <iostream>
#include "sensigolf/classic_shot_model.hpp"
int main() {
    int direction,magnitude,power;
    while(std::cin>>direction>>magnitude>>power) {
        try {
            sensigolf::ClassicShotModel shot;
            shot.reset();
            shot.set_surface_context(sensigolf::ClassicSurfaceContext{
                1,static_cast<std::uint16_t>(direction),
                static_cast<std::uint16_t>(magnitude)});
            sensigolf::ShotInput in{};
            in.club_index=12;
            in.lie_index=6;
            in.power_tick=power;
            in.accuracy_tick=63;
            in.aim_raw=0;
            shot.begin_shot(in);
            int steps=0;
            while(shot.shot_active() && steps++<256)shot.step();
            if(shot.shot_active()){std::cout<<"ERR max_ticks\n";continue;}
            const auto& state=shot.state();
            std::cout<<state.x_raw<<" "<<state.y_raw<<" "<<state.tick<<"\n";
        }catch(const std::exception& e){std::cout<<"ERR "<<e.what()<<"\n";}
    }
}
