#include <cstdint>
#include <iostream>
#include "recovered_welly_tick.hpp"
static void print_state(const pixtee::OriginalWellyTickState& s){
    const auto source_stage=s.stage==0?0u:static_cast<unsigned>(s.stage+1u);
    std::cout<<source_stage<<' '<<s.counter<<' '<<s.angle<<' '
             <<s.accumulator<<' '<<s.capture_flag<<' '
             <<s.saved_angle<<' '<<s.captured_power<<' '
             <<s.accuracy_counter<<' '<<s.step_setting<<'\n';
}
int main(){
    unsigned club;int rise,fall;
    while(std::cin>>club>>rise>>fall){
        pixtee::OriginalWellyTick meter;
        meter.start_from_original_ui_entry();
        print_state(meter.state());
        int fell_on=-1;
        for(int t=0;t<190;++t){
            if(meter.state().stage==0)break;
            const auto before=meter.state().stage;
            const bool pressed=t==rise || (fell_on>=0 && t-fell_on==fall);
            meter.step(pressed,static_cast<std::uint16_t>(club));
            print_state(meter.state());
            if(before==1 && meter.state().stage==2)fell_on=t;
        }
    }
}
