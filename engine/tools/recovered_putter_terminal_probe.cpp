#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "sensigolf/recovered_putter_terminal.hpp"

int main(int argc,char** argv) {
    if(argc!=2) {
        std::cerr<<"usage: recovered_putter_terminal_probe <landing_code>\n";
        return 2;
    }
    try {
        const auto code=static_cast<std::uint16_t>(std::strtoul(argv[1],nullptr,0));
        sensigolf::recovered::PutterTerminalState s{};
        s.counter_52=7;
        s.counter_56=11;
        s.player_mode_5a=0;
        if(!sensigolf::recovered::apply_putter_terminal(code,s)) {
            throw std::runtime_error("landing code is not a recovered putter terminal");
        }
        const char* transition=
            s.kind==sensigolf::recovered::PutterTerminalKind::Holed
                ? "hole" : "special";
        std::cout
            <<"{\"transition\":\""<<transition
            <<"\",\"player_flags\":"<<s.player_flags
            <<",\"counter_52\":"<<s.counter_52
            <<",\"counter_56\":"<<s.counter_56
            <<",\"pause\":"<<s.pause
            <<",\"terminal_flag\":"<<s.terminal_flag
            <<",\"ui_gate\":"<<static_cast<unsigned>(s.ui_gate)
            <<",\"events\":["<<s.event_id<<"]}\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr<<e.what()<<"\n";
        return 1;
    }
}
