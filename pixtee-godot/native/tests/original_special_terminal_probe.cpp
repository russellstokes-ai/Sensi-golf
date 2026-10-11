#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_putter_terminal.hpp"
int main(){
    int code,mode,flags,ct52,ct56,initial_gate;
    while(std::cin>>code>>mode>>flags>>ct52>>ct56>>initial_gate){
        sensigolf::recovered::PutterTerminalState s{};
        s.player_mode_5a=static_cast<std::uint16_t>(mode);
        s.player_flags=static_cast<std::uint16_t>(flags);
        s.counter_52=static_cast<std::uint16_t>(ct52);
        s.counter_56=static_cast<std::uint16_t>(ct56);
        s.ui_gate=static_cast<std::uint8_t>(initial_gate);
        const bool recognized=sensigolf::recovered::apply_putter_terminal(
            static_cast<std::uint16_t>(code),s);
        std::cout<<(recognized?1:0)<<" "<<s.player_flags<<" "<<s.counter_52<<" "
                 <<s.counter_56<<" "<<s.pause<<" "<<s.terminal_flag<<" "
                 <<unsigned(s.ui_gate)<<" "<<s.event_id<<" "
                 <<static_cast<int>(s.kind)<<"\n";
    }
}
