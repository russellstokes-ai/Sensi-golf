#pragma once
#include "sensigolf/authored_tree_contacts.hpp"
namespace sensigolf {
struct AuthoredBirdSample { bool active=false; std::uint32_t id=0; AuthoredTreePoint point{}; std::uint32_t member=0; };
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
inline unsigned authored_bird_group_size(std::uint32_t id) noexcept {
    if(!id)return 0;
    const auto species=(id-1)%4;
    return species==0?2u:species==1?3u:1u;
}
inline AuthoredBirdSample authored_bird_member(AuthoredBirdSample leader,unsigned member) noexcept {
    if(member>=authored_bird_group_size(leader.id)) {leader.active=false;return leader;}
    leader.member=member;
    if(member) {
        leader.point.x-=static_cast<std::int32_t>(member*12*65536);
        leader.point.y+=static_cast<std::int32_t>((member==1?8:-8)*65536);
        leader.point.z+=static_cast<std::int32_t>((member==1?1:-1)*65536);
    }
    return leader;
}
inline bool authored_bird_contact(AuthoredBirdSample before,AuthoredBirdSample after,
                                  AuthoredTreePoint from,AuthoredTreePoint to,unsigned* hit_member=nullptr) {
    if(!before.active || !after.active || before.id!=after.id) return false;
    for(unsigned member=0;member<authored_bird_group_size(after.id);++member) {
        const auto a=authored_bird_member(before,member),b=authored_bird_member(after,member);
        auto start=from,end=to;
        start.x-=a.point.x;start.y-=a.point.y;
        end.x-=b.point.x;end.y-=b.point.y;
        if(authored_airborne_contact({0,0,b.point.z},3*65536,start,end)) {
            if(hit_member)*hit_member=member;
            return true;
        }
    }
    return false;
}
}
