// Engine infrastructure tests only: all 3 course resources below are a tiny
// independently authored test fixture, NOT approved finished Hole 1,2,3.
#if defined(NDEBUG)
#undef NDEBUG
#endif
#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include "pixtee_three_hole_round.hpp"

using pixtee::PixteeThreeHoleRound;
using sensigolf::ClassicCourseResources;
using sensigolf::HoleSessionPhase;

namespace {
void be16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t w) {
    bytes.at(offset) = static_cast<std::uint8_t>(w >> 8);
    bytes.at(offset + 1u) = static_cast<std::uint8_t>(w);
}

ClassicCourseResources tiny_source_rule_cup_test_fixture() {
    // Same native-original numeric code8 cup fixture used by native C++ test;
    // this deliberately does NOT masquerade as a fully designed golf hole.
    constexpr int w = 4, h = 128;
    std::vector<std::uint8_t> map(0x60u + 2u*w*h, 0u), spt(50u, 0u);
    std::vector<std::uint8_t> descriptor(8u, 0u), selector(8u, 0u);
    be16(map, 0x54u, w);
    be16(map, 0x56u, h);
    be16(descriptor, 0u, 7u); // source descriptor 7 is numeric code8 cup
    return ClassicCourseResources(
        std::move(map),std::move(spt),std::move(descriptor),
        std::move(selector));
}

template<typename Fn> void expect_logic_error(Fn&& fn) {
    bool rejected = false;
    try {fn();}
    catch (const std::logic_error&) {rejected = true;}
    assert(rejected);
}
template<typename Fn> void expect_invalid(Fn&& fn) {
    bool rejected = false;
    try {fn();}
    catch (const std::invalid_argument&) {rejected = true;}
    assert(rejected);
}

void shoot_a_real_native_cup_event(PixteeThreeHoleRound& preview) {
    preview.set_club(12);
    preview.meter_click_raw(0);
    preview.meter_click_raw(30);
    preview.meter_click_raw(63);
    bool saw_active = false;
    for (int tick = 0; tick < 256; ++tick) {
        if (preview.hole().phase() == HoleSessionPhase::ShotActive)
            saw_active = true;
        preview.step_tick();
        if (preview.hole().phase() == HoleSessionPhase::HoleScored)
            break;
    }
    assert(saw_active);
    assert(preview.hole().phase() == HoleSessionPhase::HoleScored);
    assert(preview.hole().strokes() == 1u);
}
}  // namespace

int main() {
    expect_invalid([] {
        PixteeThreeHoleRound p({1,1,2}, {2,3,4}, 1, 2);
    });
    expect_invalid([] {
        PixteeThreeHoleRound p({0,2,3}, {2,3,4}, 1, 2);
    });
    expect_invalid([] {
        PixteeThreeHoleRound p({1,2,3}, {2,0,4}, 1, 2);
    });
    expect_invalid([] {
        PixteeThreeHoleRound p({1,2,100}, {2,3,4}, 1, 2);
    });

    PixteeThreeHoleRound preview({11,27,45}, {2,3,4}, 0x1234,0xabcd);
    assert(!preview.complete());
    assert(preview.holes_completed()==0);
    const auto unscored=preview.next_hole();
    assert(unscored && unscored->resource_id==11);
    expect_logic_error([&]{preview.commit_scored_hole();});
    const auto fixture=tiny_source_rule_cup_test_fixture();
    expect_invalid([&]{preview.register_course(46,fixture);});
    preview.register_course(11,fixture);
    preview.register_course(27,fixture);
    preview.register_course(45,fixture);

    const std::array<int,3> authored_ids{{11,27,45}};
    const std::array<int,3> authored_pars{{2,3,4}};
    for(std::size_t i=0;i<3;++i) {
        const auto requested=preview.next_hole();
        assert(requested);
        assert(requested->resource_id==authored_ids[i]);
        assert(requested->par==authored_pars[i]);
        assert(requested->round_index==i);
        preview.load_next_hole();
        expect_logic_error([&]{preview.load_next_hole();});
        expect_logic_error([&]{preview.register_course(11,fixture);});
        expect_logic_error([&]{preview.commit_scored_hole();});
        shoot_a_real_native_cup_event(preview);
        preview.commit_scored_hole();
        assert(preview.holes_completed()==i+1);
    }
    assert(preview.complete());
    assert(!preview.next_hole().has_value());
    assert(preview.underlying_original_round().holes_completed==3u);
    assert(!preview.underlying_original_round().round_complete);
    // Explicitly a PRODUCT three-hole stop; not a fabricated claim
    // that the original Windows program completed an 18-hole round.
    assert(preview.underlying_original_round().total_strokes==3u);
    assert(preview.underlying_original_round().cumulative_par==9u);
    expect_logic_error([&]{preview.load_next_hole();});
    expect_logic_error([&]{preview.commit_scored_hole();});
    expect_logic_error([&]{preview.set_club(0);});
    expect_logic_error([&]{preview.step_tick();});
    return 0;
}
