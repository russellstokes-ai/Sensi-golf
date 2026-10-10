#if defined(NDEBUG)
#undef NDEBUG // Assertions must run in GitHub's Release CTest configuration.
#endif
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "sensigolf/classic_mobile_controls.hpp"

using namespace sensigolf;

namespace {

void be16(std::vector<std::uint8_t>& bytes, std::size_t offset,
          std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value >> 8);
    bytes[offset + 1] = static_cast<std::uint8_t>(value);
}

ClassicCourseResources synthetic_green() {
    std::vector<std::uint8_t> mapm(
        ClassicCourseResources::kMapHeaderBytes + 4u * 128u * 2u, 0);
    be16(mapm, 0x54, 4);
    be16(mapm, 0x56, 128);
    std::vector<std::uint8_t> spt(50, 0);
    be16(spt, 4, 10);
    be16(spt, 6, 20);
    be16(spt, 44, 10);
    be16(spt, 46, 20);
    std::vector<std::uint8_t> descriptors(8, 0);
    std::vector<std::uint8_t> selectors(8, 0);
    be16(descriptors, 0, 7); // synthetic code-8, putter-legal green
    return ClassicCourseResources(
        std::move(mapm), std::move(spt),
        std::move(descriptors), std::move(selectors));
}

template<typename Fn>
void must_reject(Fn&& action) {
    bool rejected = false;
    try { action(); }
    catch (const std::exception&) { rejected = true; }
    assert(rejected);
}

} // namespace

int main() {
    auto course = synthetic_green();
    ClassicHoleSession hole(course, std::size_t{0});
    ClassicMobileControls controls;

    // A new Android view cannot send shots before a playable core snapshot.
    assert(!controls.enabled());
    must_reject([&] { controls.meter_click(0); });
    must_reject([&] { controls.set_aim_raw(0); });
    const bool nothing_to_dispatch = controls.dispatch_to(hole);
    assert(!nothing_to_dispatch);

    controls.sync_hole(hole);
    assert(controls.enabled());
    assert(controls.stage() == ClassicMeterStage::Idle);

    controls.set_aim_raw(4095);
    controls.nudge_aim(1);
    assert(controls.aim_raw() == 0);
    controls.nudge_aim(-1);
    assert(controls.aim_raw() == 4095);
    controls.nudge_aim(-4095);
    assert(controls.aim_raw() == 0);
    must_reject([&] { controls.set_aim_raw(4096); });

    controls.cycle_club(-1);
    assert(controls.club_index() == 12);
    controls.cycle_club(1);
    assert(controls.club_index() == 0);
    controls.set_club(12);
    must_reject([&] { controls.set_club(13); });

    // First click starts meter; second locks 105; third locks sweet spot 63.
    controls.set_aim_raw(1732);
    controls.meter_click(17);
    assert(controls.stage() == ClassicMeterStage::Power);
    must_reject([&] { controls.set_club(0); });
    must_reject([&] { controls.nudge_aim(1); });
    must_reject([&] { controls.meter_click(106); });
    assert(controls.stage() == ClassicMeterStage::Power);

    controls.meter_click(105);
    assert(controls.stage() == ClassicMeterStage::Accuracy);
    assert(!controls.queued_shot().has_value());
    controls.meter_click(63);
    assert(controls.stage() == ClassicMeterStage::ShotQueued);
    assert(controls.queued_shot().has_value());
    const auto shot = *controls.queued_shot();
    assert(shot.aim_raw == 1732);
    assert(shot.power_tick == 105);
    assert(shot.accuracy_tick == 63);
    assert(shot.club_index == 12);
    must_reject([&] { controls.meter_click(20); });
    const bool dispatched = controls.dispatch_to(hole);
    assert(dispatched);
    assert(hole.phase() == HoleSessionPhase::ShotActive);
    assert(hole.strokes() == 1);
    assert(!controls.queued_shot().has_value());
    assert(!controls.enabled());
    assert(!controls.dispatch_to(hole));
    controls.sync_hole(hole);
    assert(!controls.enabled());
    must_reject([&] { controls.meter_click(20); });

    // A brand-new playable hole enables a fresh meter; cancellation must
    // never deliver a stale power/accuracy sample to the engine.
    ClassicHoleSession another(course, std::size_t{0});
    controls.sync_hole(another);
    assert(controls.enabled());
    controls.meter_click(0);
    controls.meter_click(0);
    controls.cancel_meter();
    assert(controls.stage() == ClassicMeterStage::Idle);
    assert(!controls.queued_shot().has_value());
    controls.meter_click(0);
    controls.meter_click(5);
    controls.meter_click(105);
    assert(controls.queued_shot()->power_tick == 5);
    assert(controls.queued_shot()->accuracy_tick == 105);
    controls.reset();
    assert(!controls.enabled());
    assert(!controls.queued_shot().has_value());
    assert(controls.aim_raw() == 0);
    assert(controls.club_index() == 0);

    return 0;
}
