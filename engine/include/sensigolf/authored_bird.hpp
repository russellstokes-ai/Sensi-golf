#pragma once
#include "sensigolf/authored_tree_contacts.hpp"
namespace sensigolf {
struct AuthoredBirdSample { bool active=false; std::uint32_t id=0; AuthoredTreePoint point{}; };
// Owner-requested authored bird flight, fixed integer clock; no extra gameplay PRNG.
inline AuthoredBirdSample authored_bird_sample(std::uint64_t tick) {
    constexpr std::uint64_t period=5531, first=770, length=1680;
    const auto phase=tick%period;
    if(phase<first || phase>=first+length) return {};
    const auto progress=static_cast<std::int64_t>(phase-first);
    return {true,static_cast<std::uint32_t>(tick/period+1),
        {static_cast<std::int32_t>((-25LL*65536)+(600LL*65536*progress/length)),
         650*65536,12*65536}};
}
inline bool authored_bird_contact(AuthoredBirdSample before,AuthoredBirdSample after,
                                  AuthoredTreePoint from,AuthoredTreePoint to) {
    if(!before.active || !after.active || before.id!=after.id) return false;
    from.x-=before.point.x;from.y-=before.point.y;
    to.x-=after.point.x;to.y-=after.point.y;
    return authored_airborne_contact({0,0,after.point.z},3*65536,from,to);
}
}
