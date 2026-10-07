#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "sensigolf/classic_course_resources.hpp"

using namespace sensigolf;

namespace {

void be16(std::vector<std::uint8_t>& data, std::size_t off, std::uint16_t value) {
    data[off] = static_cast<std::uint8_t>(value >> 8);
    data[off + 1] = static_cast<std::uint8_t>(value & 0xFF);
}

ClassicCourseResources fixture() {
    // Two MAPM cells: tile 0 then tile 1.
    std::vector<std::uint8_t> mapm(0x60 + 4, 0);
    be16(mapm, 0x54, 2);
    be16(mapm, 0x56, 1);
    be16(mapm, 0x60, 0);
    be16(mapm, 0x62, 1);

    std::vector<std::uint8_t> spt(50, 0);
    for (std::size_t record = 0; record < 5; ++record) {
        for (std::size_t word = 0; word < 5; ++word) {
            be16(
                spt,
                record * 10 + word * 2,
                static_cast<std::uint16_t>(record * 100 + word));
        }
    }

    // Two eight-byte MAPI tile records. Selector is all zero, so lookup uses
    // the first big-endian word of each descriptor record.
    std::vector<std::uint8_t> desc(16, 0);
    std::vector<std::uint8_t> sel(16, 0);
    be16(desc, 0, 0x1234);  // descriptor 0x34, dir 0x200, mag 1
    be16(desc, 8, 0x214D);  // low byte >=0x4D clamps to descriptor 4

    return ClassicCourseResources(
        std::move(mapm), std::move(spt), std::move(desc), std::move(sel));
}

} // namespace

int main() {
    auto course = fixture();
    assert(course.map_width() == 2);
    assert(course.map_height() == 1);
    assert(course.mapi_tile_count() == 2);
    assert(course.map_tile(0, 0) == 0);
    assert(course.map_tile(1, 0) == 1);

    const auto a = course.lookup(0, 0, 0, 0);
    assert(a.raw_word == 0x1234);
    assert(a.descriptor_index == 0x34);
    assert(a.slope_direction == 0x0200);
    assert(a.slope_magnitude == 1);

    const auto resolved = course.resolve_surface(0, 0, 0, 0);
    assert(resolved.descriptor_index == 0x34);
    assert(resolved.landing_code == 4);
    assert(resolved.profile_slot == 3);
    assert(resolved.name == "SEMI ROUGH D2");
    assert(resolved.slope_direction == 0x0200);
    assert(resolved.slope_magnitude == 1);
    assert(resolved.product_supported);

    const auto b = course.lookup(1, 0, 0, 0);
    assert(b.raw_word == 0x214D);
    assert(b.descriptor_index == 4);
    assert(b.slope_direction == 0x0100);
    assert(b.slope_magnitude == 2);

    const auto spt = course.spt_record(3);
    assert(spt.words[0] == 300);
    assert(spt.words[4] == 304);

    bool threw = false;
    try {
        (void)course.map_tile(2, 0);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    assert(threw);

    threw = false;
    try {
        (void)course.spt_record(5);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    assert(threw);

    // Truncated MAPM.
    threw = false;
    try {
        std::vector<std::uint8_t> mapm(0x60, 0);
        be16(mapm, 0x54, 1);
        be16(mapm, 0x56, 1);
        std::vector<std::uint8_t> spt_data(50, 0);
        std::vector<std::uint8_t> bank(8, 0);
        ClassicCourseResources bad(mapm, spt_data, bank, bank);
        (void)bad;
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    // MAPM tile outside supplied MAPI bank.
    threw = false;
    try {
        std::vector<std::uint8_t> mapm(0x62, 0);
        be16(mapm, 0x54, 1);
        be16(mapm, 0x56, 1);
        be16(mapm, 0x60, 3);
        std::vector<std::uint8_t> spt_data(50, 0);
        std::vector<std::uint8_t> bank(8, 0);
        ClassicCourseResources bad(mapm, spt_data, bank, bank);
        (void)bad;
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    return 0;
}
