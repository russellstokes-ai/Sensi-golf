#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_flight.hpp"
int main() {
    int club,lie,power,accuracy,heading;
    while(std::cin>>club>>lie>>power>>accuracy>>heading) {
        try {
            sensigolf::recovered::LaunchInput input{};
            input.club_index=static_cast<std::uint16_t>(club);
            input.lie_index=static_cast<std::uint16_t>(lie);
            input.captured_power=power;
            input.accuracy_tick=accuracy;
            input.player_direction=static_cast<std::uint16_t>(heading);
            const auto s=sensigolf::recovered::launch_normal_shot(input);
            std::cout<<s.x<<" "<<s.y<<" "<<s.height<<" "
                     <<s.vertical_force<<" "<<s.horizontal_force<<" "
                     <<s.direction<<" "<<s.swing_adjuster<<" "
                     <<s.adjusted_power<<"\n";
        } catch (const std::exception& e) {
            std::cout<<"ERR "<<e.what()<<"\n";
        }
    }
}
