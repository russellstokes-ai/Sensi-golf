#include "sensigolf/authored_tree_contacts.hpp"
#include <algorithm>
#include <cstdlib>
#include <optional>
#include <set>
#include <stdexcept>
namespace sensigolf {
void validate_authored_trees(const std::vector<AuthoredTreeVolume>& trees) {
    if(trees.size()>128) throw std::invalid_argument("too many authored tree volumes");
    std::set<std::uint32_t> ids;
    for(const auto& t:trees) {
        if(t.id==0 || !ids.insert(t.id).second || t.x<0 || t.x>4096*65536 ||
           t.y<0 || t.y>4096*65536 || t.trunk_radius<65536 || t.trunk_radius>32*65536 ||
           t.canopy_radius<t.trunk_radius || t.canopy_radius>64*65536 ||
           t.trunk_height<=0 || t.canopy_bottom<0 || t.canopy_bottom>t.trunk_height ||
           t.canopy_top<t.trunk_height || t.canopy_top>128*65536)
            throw std::invalid_argument("invalid authored tree volume");
    }
}
namespace {
std::int64_t floor_div(std::int64_t n,std::int64_t d) {
    if(d<0) {n=-n;d=-d;}
    return n/d-(n%d<0?1:0);
}
std::int64_t ceil_div(std::int64_t n,std::int64_t d) {
    if(d<0) {n=-n;d=-d;}
    return n/d+(n%d>0?1:0);
}
std::optional<std::int64_t> cylinder(AuthoredTreePoint a,AuthoredTreePoint b,const AuthoredTreeVolume& tree,
              std::int32_t radius,std::int32_t bottom,std::int32_t top) {
    // Clip the swept segment to the volume's vertical interval, then check the
    // closest horizontal point. Q16 t avoids floating-point platform variance.
    std::int64_t lo=0,hi=65536;
    const std::int64_t dz=std::int64_t(b.z)-a.z;
    if(dz==0) {if(a.z<bottom || a.z>top)return std::nullopt;}
    else {
        const auto first=ceil_div((std::int64_t(dz>0?bottom:top)-a.z)*65536,dz);
        const auto last=floor_div((std::int64_t(dz>0?top:bottom)-a.z)*65536,dz);
        lo=std::max(lo,first);hi=std::min(hi,last);
        if(lo>hi)return std::nullopt;
    }
    const std::int64_t dx=std::int64_t(b.x)-a.x,dy=std::int64_t(b.y)-a.y;
    // The real native tick cannot cross 64 world units per axis. Bound products
    // explicitly, rather than permitting overflow for malformed external input.
    if(std::abs(dx)>64*65536 || std::abs(dy)>64*65536)
        throw std::out_of_range("authored contact segment exceeds native tick bound");
    const std::int64_t denominator=dx*dx+dy*dy;
    const std::int64_t dot=(std::int64_t(tree.x)-a.x)*dx+(std::int64_t(tree.y)-a.y)*dy;
    std::int64_t t=lo;
    if(denominator>0) {
        if(dot>=denominator)t=hi;
        else if(dot>0)t=std::clamp(dot*65536/denominator,lo,hi);
    }
    const auto within=[&](std::int64_t at) {
        const auto x=std::int64_t(a.x)+dx*at/65536-tree.x;
        const auto y=std::int64_t(a.y)+dy*at/65536-tree.y;
        return x*x+y*y<=std::int64_t(radius)*radius;
    };
    if(!within(t))return std::nullopt;
    // Distance decreases up to the closest point. Find first entry using
    // fixed-point bisection, retaining identical integer results on each host.
    while(lo<t) {
        const auto mid=lo+(t-lo)/2;
        if(within(mid))t=mid;else lo=mid+1;
    }
    return lo;
}
}
AuthoredTreeContact authored_tree_contact(const std::vector<AuthoredTreeVolume>& trees,
                                         AuthoredTreePoint a,AuthoredTreePoint b) {
    for(const auto p:{a,b})if(std::abs(std::int64_t(p.x))>4096*65536 || std::abs(std::int64_t(p.y))>4096*65536)
        throw std::out_of_range("tree contact point outside supported world bounds");
    AuthoredTreeContact best{};
    std::int64_t first=65537;
    for(const auto& tree:trees) {
        const auto consider=[&](std::optional<std::int64_t> entry,AuthoredTreeContactKind kind) {
            if(entry && (*entry<first || (*entry==first && tree.id<best.tree_id))) {
                first=*entry;best={tree.id,kind};
            }
        };
        // Stable tie: lower tree ID, then trunk before canopy.
        consider(cylinder(a,b,tree,tree.trunk_radius,0,tree.trunk_height),AuthoredTreeContactKind::Trunk);
        consider(cylinder(a,b,tree,tree.canopy_radius,tree.canopy_bottom,tree.canopy_top),AuthoredTreeContactKind::Canopy);
    }
    return best;
}
}
