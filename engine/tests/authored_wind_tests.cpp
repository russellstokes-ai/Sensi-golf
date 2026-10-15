#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <tuple>
#include "sensigolf/authored_wind.hpp"
#include "sensigolf/recovered_flight.hpp"
using namespace sensigolf;
using namespace sensigolf::recovered;
namespace {
auto fields(const FlightState& s) {return std::make_tuple(s.x,s.y,s.height,s.vertical_force,s.horizontal_force,s.direction,s.swing_adjuster,s.adjusted_power);}
FlightState flight(AuthoredWindState wind,int power=105) {
    auto s=launch_normal_shot({0,3,power,63,2048,0,0});
    AuthoredWindAccumulator residual{};
    for(int tick=0;tick<256;++tick) {
        apply_authored_wind(s,wind,residual,false);
        if(step_clear_air(s)==AirborneStepResult::NeedsLandingResolution)return s;
    }
    assert(false);return s;
}
}
int main() {
    auto source=launch_normal_shot({0,3,105,63,2048,0,0});
    auto calm=source;AuthoredWindAccumulator residual{};
    for(int tick=0;tick<80;++tick) {
        apply_authored_wind(calm,{true,1024,0,96},residual,false);
        assert(fields(calm)==fields(source));
        auto a=step_clear_air(calm),b=step_clear_air(source);
        assert(a==b && fields(calm)==fields(source));
        if(a==AirborneStepResult::NeedsLandingResolution)break;
    }
    auto baseline=flight({});
    auto east=flight({true,1024,80,96});auto west=flight({true,3072,80,96});
    assert(east.x>0 && west.x<0);
    assert(std::llabs(static_cast<long long>(east.x)+west.x)<65536);
    double ratio=static_cast<double>(east.x)/std::abs(baseline.y);
    std::printf("AUTHORED_WIND_CALIBRATION 8mph drift=%.6f carry=%d\n",ratio,baseline.y);
    assert(ratio>=0.05 && ratio<=0.10);
    auto strong=flight({true,1024,150,96});auto light=flight({true,1024,40,96});
    assert(light.x<east.x && east.x<strong.x);
    assert(static_cast<double>(strong.x)/std::abs(baseline.y)<0.22);
    assert(std::abs(flight({true,0,80,96}).y)<std::abs(baseline.y));
    assert(std::abs(flight({true,2048,80,96}).y)>std::abs(baseline.y));
    assert(flight({true,1024,80,96},30).x<east.x);
    assert(fields(flight({true,1024,80,96}))==fields(east));
    FlightState grounded{};grounded.horizontal_force=10000;auto original=grounded;
    apply_authored_wind(grounded,{true,1024,150,96},residual,false);
    assert(fields(grounded)==fields(original));
    bool rejected=false;try {validate_authored_wind({true,4096,80,96});}catch(...){rejected=true;}assert(rejected);
    rejected=false;try {validate_authored_wind({true,1024,151,96});}catch(...){rejected=true;}assert(rejected);
    std::puts("AUTHORED_WIND_PASS: zero-wind exact parity; symmetric measured drift; head/tail; grounded guard and replay");
}
