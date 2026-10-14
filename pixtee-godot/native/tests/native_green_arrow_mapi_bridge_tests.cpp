#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <iterator>
#include <string>
#ifdef _WIN32
#include <crtdbg.h>
#include <cstdlib>
#endif

#include "pixtee_round_runtime.hpp"
#include "pixtee_course_adapter.hpp"

namespace {
pixtee::PixteeRoundRuntime preview() {
    std::array<std::uint8_t,18> order{};
    order.fill(1u);
    std::array<std::uint8_t,100> pars{};
    pars[1]=4u;
    return pixtee::PixteeRoundRuntime(order,pars,0x1234,0xabcd);
}
} // namespace

int main(int argc,char** argv) {
#ifdef _WIN32
    _CrtSetReportMode(_CRT_ASSERT,_CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT,_CRTDBG_FILE_STDERR);
    _set_abort_behavior(0,_WRITE_ABORT_MSG|_CALL_REPORTFAULT);
#endif
    // The source-authored fine-map transfer is tested at ALL verified 16x16
    // slope direction and strength combinations. Godot must never guess
    // these values from a screenshot or one arrow sprite orientation.
    for(int phase=1;phase<=2;++phase) {
        auto recovery=preview();recovery.register_course(1,pixtee::make_lakewood_hole_1_with_slope(0,0));recovery.load_next_hole();
        const auto stable_replay=recovery.serialize_stable_replay();
        recovery.start_touch_meter();
        if(phase==2)recovery.tick_touch_meter(true);
        recovery.restore_replay(stable_replay);
        assert(recovery.touch_meter().stage==0);
    }
    auto empty=preview();
    assert(empty.verified_native_green_arrows().empty());
    for(std::uint8_t direction=0;direction<16;++direction) {
        for(std::uint8_t magnitude=0;magnitude<16;++magnitude) {
            auto session=preview();
            const auto source=pixtee::make_lakewood_hole_1_with_slope(
                direction,magnitude);
            session.register_course(1u,source);
            session.load_next_hole();
            const auto origin=session.active_green_region();
            assert(origin && origin->origin_x==96u && origin->origin_y==40u);
            const auto arrows=session.verified_native_green_arrows(8u);
            if(magnitude==0) {
                assert(arrows.empty());
                continue;
            }
            assert(!arrows.empty() && arrows.size()<2048u);
            for(const auto& arrow:arrows) {
                assert(arrow.landing_code==1u);
                assert(arrow.slope_direction_raw==
                       static_cast<std::uint16_t>(direction)*256u);
                assert(arrow.slope_magnitude==magnitude);
                assert(arrow.world_x>=96 && arrow.world_x<224);
                assert(arrow.world_y>=40 && arrow.world_y<144);
                // Original v1.014 scaled MAPS x/y at 2x within the marker
                // rectangle. Every arrow must be exactly reproducible by
                // the recovered native C++ MAPI fine-cell lookup.
                const auto resolved=source.resolve_integer_position(
                    static_cast<std::int16_t>(
                        (arrow.world_x-origin->origin_x)*2),
                    static_cast<std::int16_t>(
                        (arrow.world_y-origin->origin_y)*2),true);
                assert(resolved.landing_code==arrow.landing_code);
                assert(resolved.slope_direction==arrow.slope_direction_raw);
                assert(resolved.slope_magnitude==arrow.slope_magnitude);
            }
            bool bad=false;
            try {
                (void)session.verified_native_green_arrows(0u);
            } catch(const std::invalid_argument&) {bad=true;}
            assert(bad);
        }
    }
    if(argc==2) {
        const std::string root=argv[1];
        const auto read=[&](const char* name) {
            std::ifstream input(root+"/"+name,std::ios::binary);assert(input.good());
            return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(input),{});
        };
        sensigolf::ClassicCourseResources real(read("MAPM.bin"),read("SPT.bin"),
            read("MAPI-descriptors.bin"),read("MAPI-selectors.bin"),read("MAPS.bin"));
        auto session=preview();session.register_course(1,real);session.load_next_hole();
        const auto arrows=session.verified_native_green_arrows(16);
        assert(!arrows.empty());
        session.set_original_player_mode_5a(0);
        session.set_verified_flag_object(true);
        session.set_authored_birds_enabled(true);
        session.set_authored_tree_objects({{1,404*65536,88*65536,2*65536,15*65536,20*65536,11*65536,44*65536}});
        auto touch_session=preview();touch_session.register_course(1,real);touch_session.load_next_hole();touch_session.set_aim_raw(1946);
        touch_session.start_touch_meter();
        for(int i=0;i<100;++i)touch_session.tick_touch_meter(false);
        touch_session.tick_touch_meter(true);
        auto touch_copy=preview();touch_copy.register_course(1,real);
        touch_copy.restore_native_session(touch_session.serialize_native_session());
        assert(touch_copy.native_fingerprint()==touch_session.native_fingerprint());
        while(touch_session.touch_meter().position>160) {
            touch_session.tick_touch_meter(false);touch_copy.tick_touch_meter(false);
            assert(touch_copy.native_fingerprint()==touch_session.native_fingerprint());
        }
        assert(touch_session.tick_touch_meter(true));assert(touch_copy.tick_touch_meter(true));
        for(int i=0;i<2048;++i){touch_session.step_tick();touch_copy.step_tick();}
        assert(touch_copy.native_fingerprint()==touch_session.native_fingerprint());
        touch_session.cancel_meter();
        session.start_original_welly();
        for(int i=0;i<30;++i) {session.tick_original_welly(false);session.step_tick();}
        auto resumed=preview();resumed.register_course(1,real);
        resumed.restore_native_session(session.serialize_native_session());
        assert(resumed.native_fingerprint()==session.native_fingerprint());
        while(session.original_welly_state().stage==1) {
            const bool press=session.original_welly_state().counter==50;
            session.tick_original_welly(press);resumed.tick_original_welly(press);
        }
        while(session.original_welly_state().counter>64) {
            session.tick_original_welly(false);resumed.tick_original_welly(false);
        }
        session.tick_original_welly(true);resumed.tick_original_welly(true);
        for(int i=0;i<10;++i)session.step_tick();
        resumed.restore_native_session(session.serialize_native_session());
        for(int i=0;i<100;++i) {session.step_tick();resumed.step_tick();assert(resumed.native_fingerprint()==session.native_fingerprint());}
        const auto good=resumed.native_fingerprint();
        bool rejected=false;
        try {resumed.restore_native_session(session.serialize_native_session()+"corrupt");}catch(const std::invalid_argument&){rejected=true;}
        assert(rejected && resumed.native_fingerprint()==good);

        const auto origin=real.green_region();assert(origin);
        for(const auto& arrow:arrows) {
            assert(origin->contains_course(arrow.world_x,arrow.world_y));
            assert(arrow.slope_direction_raw==0 && arrow.slope_magnitude==1);
        }
    }
    return 0;
}
