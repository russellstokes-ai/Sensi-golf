#include "pixtee_course_adapter.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace pixtee {
namespace {
constexpr int kMapW=19, kMapH=64, kGreenW=16, kGreenH=26;
constexpr int kGreenOriginX=96, kGreenOriginY=40;
constexpr std::uint16_t kRegionMarker=0xE000u;

void be16(std::vector<std::uint8_t>& bytes, std::size_t off, std::uint16_t v) {
    bytes.at(off)=static_cast<std::uint8_t>(v >> 8);
    bytes.at(off+1)=static_cast<std::uint8_t>(v);
}
bool ellipse(double x,double y,double cx,double cy,double rx,double ry) {
    const auto dx=(x-cx)/rx,dy=(y-cy)/ry;
    return dx*dx+dy*dy < 1.;
}
bool creek(double x,double y) {
    // Existing independently authored Lakewood winding creek outline.
    constexpr double points[][2]={
        {233,45},{226,68},{230,95},{225,126},{238,161},{231,195},
        {238,232},{224,268},{233,308},{239,345},{231,382},
        {242,422},{299,430},{299,45}};
    bool inside=false;
    for(int i=0,j=13;i<14;j=i++) {
        const auto &a=points[i],&b=points[j];
        if((a[1]>y)!=(b[1]>y) &&
           x<(b[0]-a[0])*(y-a[1])/(b[1]-a[1])+a[0]) inside=!inside;
    }
    return inside;
}

// Independently authored Pixtee surfaces, but every resulting descriptor is
// encoded using the *actual recovered Sensible Golf* 77-entry surface table.
std::uint8_t surface_at(double x,double y,bool green_detail) {
    if (ellipse(x,y,142.,82.,30.,30.*.82)) return 31; // GREEN H4, code 1
    if (green_detail) return 0;                        // original skirt code 2
    if (creek(x,y)) return 6;                          // original water code 35
    if (ellipse(x,y,91,141,19,20) ||
        ellipse(x,y,189,133,20,21) ||
        ellipse(x,y,200,289,23,21)) return 5;          // original sand code 7
    if (ellipse(x,y,150,458,14,14)) return 1;          // tee fairway profile
    const double travel=std::clamp((458.-y)/376.,0.,1.);
    const double center=150.-8.*travel-18.*std::sin(travel*3.141592653589793);
    if(y>=82. && y<=463. && std::abs(x-center)<43.) return 1;
    return 3;                                          // original rough code 5
}

struct BankBuilder {
    std::vector<std::uint8_t> descriptor_bank, selector_bank;
    std::unordered_map<std::string,std::uint16_t> tile_ids;

    std::uint16_t tile(const std::array<std::uint16_t,32>& samples) {
        // MAPI contains four candidate 16-bit words per tile and two selector
        // bitplanes for an 8x4 grid of 2x2 source-world-unit collision cells.
        std::array<std::uint16_t,4> words{};
        std::array<std::uint8_t,8> selectors{};
        unsigned distinct=0;
        for(unsigned row=0;row<4;++row) {
            for(unsigned col=0;col<8;++col) {
                const auto sample=samples[row*8+col];
                unsigned which=0;
                while(which<distinct && words[which]!=sample) ++which;
                if(which==distinct) {
                    if(distinct==4) {
                        throw std::runtime_error(
                            "Lakewood tile exceeds recovered 4-surface MAPI encoding;"
                            " author a finer boundary or tile seam, do NOT discard collision data");
                    }
                    words[distinct++]=sample;
                }
                const auto bit=static_cast<std::uint8_t>(0x80u>>col);
                if((which&1u)!=0) selectors[row]|=bit;
                if((which&2u)!=0) selectors[row+4]|=bit;
            }
        }
        std::string key;
        key.reserve(16);
        for(auto w:words) {
            key.push_back(static_cast<char>(w>>8));
            key.push_back(static_cast<char>(w));
        }
        for(auto m:selectors) key.push_back(static_cast<char>(m));
        const auto present=tile_ids.find(key);
        if(present!=tile_ids.end()) return present->second;
        // MAPM low 10 bits reference a tile bank; original parser masks 0x03ff.
        if(tile_ids.size()>=1024) {
            throw std::runtime_error(
                "Lakewood distinct MAPI patterns exceed original 10-bit tile ID");
        }
        const auto id=static_cast<std::uint16_t>(tile_ids.size());
        tile_ids.emplace(key,id);
        for(auto w:words) {
            descriptor_bank.push_back(static_cast<std::uint8_t>(w>>8));
            descriptor_bank.push_back(static_cast<std::uint8_t>(w));
        }
        selector_bank.insert(selector_bank.end(),selectors.begin(),selectors.end());
        return id;
    }
};

std::vector<std::uint8_t> rasterize(
    BankBuilder& bank,int width,int height,bool green_mode,
    std::uint8_t magnitude,std::uint8_t direction) {
    std::vector<std::uint8_t> map(0x60u+static_cast<std::size_t>(width*height)*2u,0);
    be16(map,0x54,static_cast<std::uint16_t>(width));
    be16(map,0x56,static_cast<std::uint16_t>(height));
    for(int yy=0;yy<height;++yy) {
        for(int xx=0;xx<width;++xx) {
            std::array<std::uint16_t,32> cells{};
            for(int sy=0;sy<4;++sy) {
                for(int sx=0;sx<8;++sx) {
                    const double local_x=16*xx+2*sx+1;
                    const double local_y=8*yy+2*sy+1;
                    const double wx=green_mode
                        ? kGreenOriginX+local_x*.5 : local_x;
                    const double wy=green_mode
                        ? kGreenOriginY+local_y*.5 : local_y;
                    const auto surface=surface_at(wx,wy,green_mode);
                    // Original 16-bit MAPI schema: high nibble magnitude,
                    // next nibble direction (16 sectors), low byte descriptor.
                    const std::uint16_t slope=green_mode && surface==31
                        ? static_cast<std::uint16_t>(
                            (magnitude<<12)|(direction<<8)) : 0;
                    cells[sy*8+sx]=static_cast<std::uint16_t>(slope|surface);
                }
            }
            auto tile=bank.tile(cells);
            // Original green transition routine scans for at least 3 markers;
            // only the first marker determines the course-to-green origin.
            // Exact marker placement is Pixtee-authored, not copied.
            if(!green_mode && yy==kGreenOriginY/8 &&
               xx>=kGreenOriginX/16 && xx<kGreenOriginX/16+3) {
                tile=static_cast<std::uint16_t>(tile|kRegionMarker);
            }
            be16(map,0x60u+2u*static_cast<std::size_t>(yy*width+xx),tile);
        }
    }
    return map;
}
} // namespace

sensigolf::ClassicCourseResources make_lakewood_hole_1_with_slope(
    std::uint8_t direction, std::uint8_t magnitude) {
    if(direction>15 || magnitude>15)
        throw std::invalid_argument("Original MAPI slope nibbles must each be 0..15");
    BankBuilder bank;
    auto map=rasterize(bank,kMapW,kMapH,false,0,0);
    auto maps=rasterize(bank,kGreenW,kGreenH,true,magnitude,direction);
    std::vector<std::uint8_t> spt(50,0);
    for(unsigned player=0;player<4;++player) {
        be16(spt,player*10u+4u,150);
        be16(spt,player*10u+6u,458);
    }
    be16(spt,44,142);
    be16(spt,46,82);
    // Actual recovered parser, lookup, coordinate conversion and ball/slope
    // response are untouched. This factory only authors independent course
    // geometry and supplies the original-format structured collision data.
    return sensigolf::ClassicCourseResources(
        std::move(map),std::move(spt),
        std::move(bank.descriptor_bank),std::move(bank.selector_bank),
        std::move(maps));
}
sensigolf::ClassicCourseResources make_lakewood_hole_1() {
    // Until a green layout is artist-authored / owner-approved, do not invent
    // arbitrary hill vectors. The separate slope-enabled fixture exercises
    // the recovered direction and magnitude pipeline with exact raw values.
    return make_lakewood_hole_1_with_slope(0,0);
}
} // namespace pixtee
