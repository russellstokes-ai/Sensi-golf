#include "recovered_welly_tick.hpp"
#include <iostream>
int main() {
    int club,rise,fall;
    while(std::cin>>club>>rise>>fall) {
        pixtee::OriginalWellyTick meter;
        meter.reset();
        int falling_at=-1;
        for(int t=0;t<185;++t) {
            const auto old=meter.state().stage;
            if(old==0) break;
            const bool pressed=(t==rise)||
                (falling_at>=0 && t-falling_at==fall);
            meter.step(pressed,static_cast<std::uint16_t>(club));
            const auto &s=meter.state();
            std::cout<<club<<" "<<rise<<" "<<fall<<" "<<t<<" "
                <<s.stage<<" "<<s.counter<<" "<<s.captured_power<<" "
                <<s.accuracy_counter<<" "<<s.capture_flag<<" "<<s.angle<<" "
                <<s.step_setting<<" "<<s.saved_angle<<"\n";
            if(old==1 && s.stage==2) falling_at=t;
        }
    }
}
