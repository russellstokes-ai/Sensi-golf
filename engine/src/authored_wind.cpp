#include "sensigolf/authored_wind.hpp"
#include <cstdint>
// Flight and legacy math headers declare overlapping constants; reuse the
// existing projection function without importing those unrelated declarations.
namespace sensigolf::recovered {std::int32_t project_q14(std::int32_t,std::int16_t) noexcept;}
#include <limits>
#include <stdexcept>
#include <cstdlib>
namespace sensigolf {
namespace {
std::uint64_t root(std::uint64_t n) {
    std::uint64_t out=0,bit=std::uint64_t{1}<<62;
    while(bit>n)bit>>=2;
    while(bit) {if(n>=out+bit){n-=out+bit;out=(out>>1)+bit;}else out>>=1;bit>>=2;}
    return out;
}
std::int32_t checked(std::int64_t n) {
    if(n<INT32_MIN||n>INT32_MAX)throw std::overflow_error("authored wind force overflow");
    return static_cast<std::int32_t>(n);
}
}
void validate_authored_wind(const AuthoredWindState& w) {
    if(w.toward_raw>=4096||w.speed_mph_x10>150||w.response<32||w.response>192)
        throw std::invalid_argument("wind direction/speed/model outside authored range");
}
void apply_authored_wind(recovered::FlightState& f,const AuthoredWindState& w,
                        AuthoredWindAccumulator& a,bool green) {
    if(!w.enabled||w.speed_mph_x10==0)return;
    // No ground/putter force: only ticks that can remain airborne qualify.
    if(static_cast<std::int64_t>(f.height)+f.vertical_force-recovered::kGravityPerTick<=0)return;
    const int scale=green?2:1;
    if(a.coordinate_scale!=scale) {
        a.residual_x=checked(static_cast<std::int64_t>(a.residual_x)*scale/a.coordinate_scale);
        a.residual_y=checked(static_cast<std::int64_t>(a.residual_y)*scale/a.coordinate_scale);
        a.fraction_x=a.fraction_x*scale/a.coordinate_scale;
        a.fraction_y=a.fraction_y*scale/a.coordinate_scale;
        a.coordinate_scale=static_cast<std::uint8_t>(scale);
    }
    constexpr std::int64_t denominator=10*16384;
    const auto impulse=[&](std::uint16_t heading,std::int64_t& fraction) {
        fraction+=static_cast<std::int64_t>(w.response)*w.speed_mph_x10*scale*recovered::trig_q14(heading);
        const auto part=fraction/denominator;fraction-=part*denominator;return part;
    };
    const std::int32_t x=checked(static_cast<std::int64_t>(recovered::project_q14(f.horizontal_force,recovered::trig_q14(f.direction)))+a.residual_x+impulse(w.toward_raw,a.fraction_x));
    const std::int32_t y=checked(static_cast<std::int64_t>(recovered::project_q14(f.horizontal_force,recovered::trig_q14(static_cast<std::uint16_t>(f.direction+1024))))+a.residual_y+impulse(static_cast<std::uint16_t>(w.toward_raw+1024),a.fraction_y));
    const auto magnitude=root(static_cast<std::uint64_t>(static_cast<std::int64_t>(x)*x)+static_cast<std::uint64_t>(static_cast<std::int64_t>(y)*y));
    f.horizontal_force=checked(static_cast<std::int64_t>(magnitude));
    std::int64_t best=std::numeric_limits<std::int64_t>::min();
    std::uint16_t direction=f.direction;
    // Integer reference table keeps all platforms and replay paths identical.
    for(unsigned heading=0;heading<4096;++heading) {
        const auto dot=static_cast<std::int64_t>(x)*recovered::trig_q14(static_cast<std::uint16_t>(heading))+
            static_cast<std::int64_t>(y)*recovered::trig_q14(static_cast<std::uint16_t>(heading+1024));
        if(dot>best){best=dot;direction=static_cast<std::uint16_t>(heading);}
    }
    f.direction=direction;
    a.residual_x=x-recovered::project_q14(f.horizontal_force,recovered::trig_q14(direction));
    a.residual_y=y-recovered::project_q14(f.horizontal_force,recovered::trig_q14(static_cast<std::uint16_t>(direction+1024)));
}
}
