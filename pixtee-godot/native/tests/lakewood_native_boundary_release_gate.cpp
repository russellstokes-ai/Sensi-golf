// Commercial release regression: actual 544x912 Lakewood-world dimensions,
// source-verified native physics, source original out-of-world extra stroke,
// recovery, separate IN-MAP OutOfBoundsTile and water. No UI fake physics.
//
// This is a controlled uniform-terrain fixture at real world dimensions.
// It does NOT certify the final author's polygons or tree collisions.
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>
#include "sensigolf/classic_course_resources.hpp"
#include "sensigolf/classic_hole_session.hpp"
#include "sensigolf/recovered_world_boundary.hpp"

namespace {
constexpr int kWorldW=544, kWorldH=912, kMapW=34, kMapH=114;
static_assert(kWorldW==kMapW*16 && kWorldH==kMapH*8,
              "Lakewood world must match source tile units");

void be16(std::vector<std::uint8_t>& bytes, std::size_t index, std::uint16_t val) {
    bytes.at(index)=static_cast<std::uint8_t>(val>>8);
    bytes.at(index+1)=static_cast<std::uint8_t>(val&255u);
}

sensigolf::ClassicCourseResources course(std::uint16_t descriptor) {
    std::vector<std::uint8_t> map(0x60u + kMapW*kMapH*2u,0u);
    be16(map,0x54,kMapW);
    be16(map,0x56,kMapH);
    std::vector<std::uint8_t> spt(50u,0u);
    // 4 real native start slots. Cup away from tested boundary shot.
    for(std::size_t i=0;i<4u;++i) {
        be16(spt,i*10u+4u,272);
        be16(spt,i*10u+6u,880);
    }
    be16(spt,44u,272);
    be16(spt,46u,128);
    std::vector<std::uint8_t> words(8u,0u), selector(8u,0u);
    be16(words,0u,descriptor);
    return sensigolf::ClassicCourseResources(
        std::move(map),std::move(spt),std::move(words),std::move(selector));
}
void finish_shot(sensigolf::ClassicHoleSession& hole, int max_ticks=4096) {
    int ticks=0;
    while(hole.phase()==sensigolf::HoleSessionPhase::ShotActive && ticks++<max_ticks)
        hole.step();
    assert(ticks<max_ticks);
}
sensigolf::ClassicShotRequest shot(int club=0, int power=105) {
    sensigolf::ClassicShotRequest input{};
    input.club_index=static_cast<std::uint16_t>(club);
    input.aim_raw=0; // Native heading 0 advances course Y, towards south border
    input.power_tick=power;
    input.accuracy_tick=63;
    return input;
}
void assert_inside_after_recovery(const sensigolf::ClassicHoleSession& h) {
    assert(h.ball_x_raw()>=0 && h.ball_x_raw() < (kWorldW<<16));
    assert(h.ball_y_raw()>=0 && h.ball_y_raw() < (kWorldH<<16));
}
} // namespace

int main() {
    // Actual source OOB dimensions and all four edges, no viewport dependency.
    sensigolf::recovered::OriginalWorldBoundaryInput boundary{};
    boundary.x_extent=kWorldW-0x100;
    boundary.y_extent=kWorldH-0xD0;
    for(const auto& p: {std::pair<int,int>{0,0},{272,880},{kWorldW-1,kWorldH-1}}) {
        boundary.ball_x=p.first<<16;
        boundary.ball_y=p.second<<16;
        assert(!sensigolf::recovered::original_world_boundary_check(boundary).outside_world);
    }
    for(const auto& p: {std::pair<int,int>{-1,88},{0,-1},{kWorldW,88},{88,kWorldH},
                       {kWorldW,kWorldH},{-1,-1}}) {
        boundary.ball_x=p.first*65536;
        boundary.ball_y=p.second*65536;
        const auto result=sensigolf::recovered::original_world_boundary_check(boundary);
        assert(result.outside_world);
        assert(result.source_map_ax==-1); // source-owned negative MAPI lookup
        assert(result.requires_out_of_bounds_dispatch);
        assert(result.strokes_on_hole==1); // source extra penalty, not second launch
        assert(result.strokes_total==1);
        assert(result.ball_pause==100);
    }

    // Real C++ *shot and state transition* near 912-unit lower edge.
    // This uses original fixed-tick ball/terrain and session state, not a
    // synthetic call to the isolated OriginalWorldBoundary helper alone.
    const auto fairway=course(1); // source descriptor 1 is fairway
    sensigolf::ClassicHoleSession out(fairway,272<<16,911<<16);
    out.begin_shot(shot());
    assert(out.strokes()==1); // genuine input registered
    finish_shot(out);
    assert(out.phase()==sensigolf::HoleSessionPhase::HazardStopped);
    assert(out.last_hazard_kind()==sensigolf::ClassicNamedHazard::OutsideWorld);
    assert(out.strokes()==2); // launch plus correct original WORLD OOB penalty
    assert(out.recovered_counters().player_52==2);
    assert(out.recovered_counters().player_56==2);
    const auto event=out.pending_world_oob_event();
    assert(event.has_value());
    assert(event->byte_420fef==1);
    assert(event->dword_420548==0x1000du);
    assert(out.hazard_pause_remaining()==100);
    for(int i=0;i<100;++i) {
        out.step();
    }
    assert(out.phase()==sensigolf::HoleSessionPhase::HazardStopped);
    assert(out.hazard_pause_remaining()==0);
    out.step();
    assert(out.phase()==sensigolf::HoleSessionPhase::HazardRecovered);
    assert_inside_after_recovery(out);
    // The source disallows starting a new shot until the host acknowledges
    // the real recovery event. No timer/HUD fake can skip this gate.
    bool blocked=false;
    try {out.begin_shot(shot());}
    catch(const std::logic_error&) {blocked=true;}
    assert(blocked);
    out.acknowledge_hazard_recovery();
    assert(out.phase()==sensigolf::HoleSessionPhase::ReadyForShot);
    assert(!out.pending_world_oob_event().has_value());
    assert_inside_after_recovery(out);
    out.begin_shot(shot());
    assert(out.strokes()==3);

    // In-map course descriptor OOB is NOT outside the world. Its named
    // hazard code follows separate original source terrain logic and
    // does not manufacture a world-boundary dispatch.
    auto mapped=course(22);
    sensigolf::ClassicHoleSession tile_oob(mapped,272<<16,400<<16);
    tile_oob.begin_shot(shot(5,83));
    finish_shot(tile_oob);
    assert(tile_oob.phase()==sensigolf::HoleSessionPhase::HazardStopped);
    assert(tile_oob.last_hazard_kind()==sensigolf::ClassicNamedHazard::OutOfBoundsTile);
    assert(!tile_oob.pending_world_oob_event().has_value());
    assert(tile_oob.strokes()==1);
    assert(tile_oob.hazard_pause_remaining()==100);

    // Water also uses code-35 but must NOT accidentally become OOB.
    auto wet=course(6);
    sensigolf::ClassicHoleSession water(wet,272<<16,400<<16);
    water.begin_shot(shot(5,83));
    finish_shot(water);
    assert(water.phase()==sensigolf::HoleSessionPhase::HazardStopped);
    assert(water.last_hazard_kind()==sensigolf::ClassicNamedHazard::Water);
    assert(!water.pending_world_oob_event().has_value());
    assert(water.strokes()==1);
    for(int tick=0;tick<101;++tick)water.step();
    assert(water.phase()==sensigolf::HoleSessionPhase::HazardRecovered);
    water.acknowledge_hazard_recovery();
    assert(water.phase()==sensigolf::HoleSessionPhase::ReadyForShot);
    return 0;
}
