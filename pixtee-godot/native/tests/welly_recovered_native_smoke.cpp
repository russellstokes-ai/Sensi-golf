#include <cassert>
#include <cstdint>
#ifdef _WIN32
#include <crtdbg.h>
#include <cstdlib>
#endif
#include "recovered_welly_tick.hpp"
#include "pixtee_touch_meter.hpp"
int main(){
#ifdef _WIN32
    _CrtSetReportMode(_CRT_ASSERT,_CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT,_CRTDBG_FILE_STDERR);
    _set_abort_behavior(0,_WRITE_ABORT_MSG|_CALL_REPORTFAULT);
#endif
    pixtee::TouchMeter touch;
    touch.start(55,70);
    for(int i=0;i<100;++i)touch.step(false);
    const auto position=touch.position;
    touch.step(true);
    assert(touch.stage==2 && touch.power==60 && touch.position==position);
    touch.step(false);assert(touch.position<position);
    while(touch.position>160)touch.step(false);
    assert(touch.sample()==63);
    assert(touch.step(true) && touch.stage==0);
    touch.start(55,70);const auto easy=touch.red_half;
    touch.start(60,66);assert(touch.red_half<easy);
    const auto rough=touch.red_half;
    touch.start(61,65);assert(touch.red_half<rough);
    touch.position=160-touch.red_half-2;assert(touch.sample()>63);
    touch.position=160+touch.red_half+2;assert(touch.sample()<63);
    // Exact outcomes compared with Windows v1.014 machine code; see
    // original Welly oracle CI 38159584005 (1986/1986 samples).
    pixtee::OriginalWellyTick meter;
    meter.step(false,0);
    assert(meter.state().stage==1);
    assert(meter.state().counter==1);
    assert(meter.state().angle==4095);
    for(int tick=1;tick<=104;++tick) meter.step(tick==12,0);
    assert(meter.state().stage==2);
    assert(meter.state().counter==70);
    assert(meter.state().captured_power==12);
    assert(meter.state().accuracy_counter==0);
    assert(meter.state().angle==1950);
    for(int tick=105;tick<=114;++tick) meter.step(tick==114,0);
    assert(meter.state().stage==0);
    assert(meter.state().counter==60);
    assert(meter.state().captured_power==12);
    assert(meter.state().accuracy_counter==9);
    assert(meter.state().angle==0);
    meter.reset();
    for(int tick=0;tick<=175;++tick)meter.step(false,0);
    assert(meter.state().stage==0);
    assert(meter.state().counter==65535);
    assert(meter.state().accuracy_counter==71);
    meter.reset();
    for(int tick=0;tick<=69;++tick)meter.step(false,12);
    assert(meter.state().counter==70);
    assert(meter.state().step_setting==3); // original putter caps quotient at 2
    meter.reset();
    for(int tick=0;tick<=69;++tick)meter.step(false,0);
    assert(meter.state().step_setting==2);
    return 0;
}
