#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_interactions.hpp"
int main() {
    std::uint32_t early,flags,stroke,total,pause,gate;
    while(std::cin>>early>>flags>>stroke>>total>>pause>>gate) {
        sensigolf::recovered::OriginalSpecialGreenStopState x{};
        x.player_flags=static_cast<std::uint16_t>(flags);
        x.counter_52=static_cast<std::uint16_t>(stroke);
        x.counter_56=static_cast<std::uint16_t>(total);
        x.ball_pause=static_cast<std::uint16_t>(pause);
        x.event_gate=static_cast<std::uint16_t>(gate);
        const auto y=sensigolf::recovered::original_special_green_stop_state(
            early!=0,x);
        std::cout<<y.player_flags<<' '<<y.counter_52<<' '
                 <<y.counter_56<<' '<<y.ball_pause<<' '
                 <<y.event_gate<<'\n';
    }
}
