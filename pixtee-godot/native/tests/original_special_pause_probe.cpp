#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_putter_terminal.hpp"
int main(){int counter,gate;while(std::cin>>counter>>gate){
    sensigolf::recovered::OriginalPauseTickState input{};
    input.remaining=static_cast<std::uint16_t>(counter);
    input.ui_state=static_cast<std::uint8_t>(gate);
    const auto out=sensigolf::recovered::original_special_pause_tick(input);
    std::cout<<out.remaining<<" "<<unsigned(out.ui_state)<<" "<<(out.expired?1:0)<<"\n";
}}