#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>

#include "sensigolf/recovered_course_lookup.hpp"

using namespace sensigolf::recovered;

int main() {
    std::array<std::uint8_t, 8> desc{
        0x12,0x03, 0xA4,0x06, 0x57,0x23, 0xF1,0x80
    };
    std::array<std::uint8_t, 8> mask{};

    // Neither selector plane set -> offset 0.
    auto r=lookup_course_subcell(
        desc.data(),desc.size(),mask.data(),mask.size(),0,0,0);
    assert(r.raw_word==0x1203);
    assert(r.descriptor_index==3);
    assert(r.slope_direction==0x0200);
    assert(r.slope_magnitude==1);

    // First selector plane -> offset 2.
    mask[0]=0x80;
    r=lookup_course_subcell(
        desc.data(),desc.size(),mask.data(),mask.size(),0,0,0);
    assert(r.raw_word==0xA406);
    assert(r.descriptor_index==6);
    assert(r.slope_direction==0x0400);
    assert(r.slope_magnitude==10);

    // Both selector planes -> offset 6; descriptor 0x80 clamps to 4.
    mask[4]=0x80;
    r=lookup_course_subcell(
        desc.data(),desc.size(),mask.data(),mask.size(),0,0,0);
    assert(r.raw_word==0xF180);
    assert(r.descriptor_index==4);
    assert(r.slope_direction==0x0100);
    assert(r.slope_magnitude==15);

    bool rejected=false;
    try {
        (void)lookup_course_subcell(
            desc.data(),desc.size(),mask.data(),mask.size(),0,8,0);
    } catch(const std::out_of_range&) {
        rejected=true;
    }
    assert(rejected);

    return 0;
}
