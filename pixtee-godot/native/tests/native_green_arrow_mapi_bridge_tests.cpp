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
#include <cstdio>
#ifdef _WIN32
#include <crtdbg.h>
#include <cstdlib>
#endif

#include "pixtee_round_runtime.hpp"
#include "pixtee_club_display.hpp"
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
    // HUD V2 power audit: identical inputs through mobile adapter and direct
    // recovered control produce identical ball states, for every sampled tick.
    assert(pixtee::nominal_club_ranges()[0]==240);
    for(int club: {0,6,11,12}) for(int percent: {0,25,50,75,100}) {
        auto mobile=preview(); auto direct=preview();
        const auto flat=pixtee::make_lakewood_hole_1_with_slope(0,0);
        for(auto* game: {&mobile,&direct}) {
            game->register_course(1,flat);game->load_next_hole();
            game->set_club(club);game->set_aim_raw(2048);
        }
        mobile.start_touch_meter();
        const int target=160+(1050*percent+50)/100;
        while(mobile.touch_meter().stage==1 && mobile.touch_meter().position<target)mobile.tick_touch_meter(false);
        if(mobile.touch_meter().stage==1)mobile.tick_touch_meter(true);
        const int captured=mobile.touch_meter().power;
        assert(std::abs(captured-105*percent/100)<=1);
        while(mobile.touch_meter().position>160)mobile.tick_touch_meter(false);
        assert(mobile.touch_meter().sample()==63);
        mobile.tick_touch_meter(true);
        direct.meter_click_raw(0);direct.meter_click_raw(captured);direct.meter_click_raw(63);
        for(int i=0;i<256;++i) {
            const auto& a=mobile.hole().ball_state();const auto& b=direct.hole().ball_state();
            assert(a.x_raw==b.x_raw && a.y_raw==b.y_raw && a.z_raw==b.z_raw);
            assert(a.vx_raw==b.vx_raw && a.vy_raw==b.vy_raw && a.vz_raw==b.vz_raw);
            assert(mobile.hole().strokes()==direct.hole().strokes());
            mobile.step_tick();direct.step_tick();
        }
        std::printf("HUD_POWER_AUDIT club=%d display=%d raw=%d native_equal=256_ticks\n",club,percent,captured);
    }
    for(int club: {0,6,11,12}) for(int lie=0;lie<10;++lie) for(int power: {0,26,52,78,105}) {
        const auto window=sensigolf::recovered::original_accuracy_window(club,lie);
        pixtee::TouchMeter mobile;mobile.start(window.lower,window.upper);
        mobile.position=160+power*10;mobile.step(true);mobile.position=160;
        sensigolf::recovered::LaunchInput input{};
        input.club_index=club;input.lie_index=lie;input.captured_power=mobile.power;
        input.accuracy_tick=mobile.sample();input.player_direction=2048;
        const auto launch=sensigolf::recovered::launch_normal_shot(input);
        const auto params=sensigolf::recovered::club_launch_parameters()[club];
        assert(launch.adjusted_power==power);
        assert(launch.horizontal_force==params.horizontal_base+params.power_scale*power);
        assert(launch.vertical_force==params.vertical_base+params.power_scale*power);
        mobile.position=160+mobile.red_half+mobile.yellow/2;
        input.accuracy_tick=mobile.sample();
        assert(input.accuracy_tick<63); // mirrored left yellow -> hook for right-handed golfer
        assert(sensigolf::recovered::launch_normal_shot(input).direction>2048);
        mobile.position=160-mobile.red_half-mobile.yellow/2;
        input.accuracy_tick=mobile.sample();
        assert(input.accuracy_tick>63);
        assert(sensigolf::recovered::launch_normal_shot(input).direction<2048);
    }
    std::puts("HUD_NATIVE_AUDIT_PASS: 200 club/lie/power mappings, 20 full adapter paths, hook/slice and nominal Max");
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
    {
        const auto course=pixtee::make_lakewood_hole_1_with_slope(0,0);
        auto wind=preview();wind.register_course(1,course);wind.load_next_hole();
        wind.set_authored_wind(1024,80,true);
        wind.set_aim_raw(2048);wind.meter_click_raw(0);wind.meter_click_raw(105);wind.meter_click_raw(63);
        for(int i=0;i<12;++i)wind.step_tick();
        auto calm=preview();calm.register_course(1,course);calm.load_next_hole();
        calm.set_aim_raw(2048);calm.meter_click_raw(0);calm.meter_click_raw(105);calm.meter_click_raw(63);
        for(int i=0;i<12;++i)calm.step_tick();
        assert(wind.hole().ball_state().x_raw>calm.hole().ball_state().x_raw);
        bool locked=false;try{wind.set_authored_wind(0,60,true);}catch(const std::logic_error&){locked=true;}assert(locked);
        auto restored=preview();restored.register_course(1,course);
        restored.restore_native_session(wind.serialize_native_session());
        assert(restored.authored_wind().speed_mph_x10==80);
        assert(restored.native_fingerprint()==wind.native_fingerprint());
        for(int i=0;i<40;++i){wind.step_tick();restored.step_tick();assert(restored.native_fingerprint()==wind.native_fingerprint());}
        auto invalid=preview();invalid.register_course(1,course);invalid.load_next_hole();
        bool rejected=false;try{invalid.set_authored_wind(-1,60,true);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
        rejected=false;try{invalid.set_authored_wind(1024,151,true);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
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

        bool board_seen=false;
        for(int club:{7,9,11,0})for(int power:{35,50,65,80,100}) {
            if(board_seen)break;
            auto probe=preview();probe.register_course(1,real);probe.load_next_hole();
            probe.set_original_player_mode_5a(0);probe.set_verified_flag_object(true);probe.set_soft_ground_enabled(true);
            probe.set_authored_tree_objects({{1,26476544,5767168,131072,1703936,1179648,1048576,2949120},{2,16121856,7471104,131072,1769472,1179648,1048576,3014656},{3,6422528,8847360,131072,1835008,1310720,1114112,3211264},{4,30277632,11534336,131072,1835008,1310720,1114112,3211264},{5,12517376,17432576,131072,1572864,1114112,983040,2752512},{6,29229056,19398656,131072,1966080,1310720,1179648,3342336},{7,19333120,21495808,131072,1572864,1114112,983040,2752512},{8,5111808,22937600,131072,1638400,1114112,983040,2818048},{9,31195136,23527424,131072,1835008,786432,1114112,3145728},{10,10878976,26345472,131072,1638400,1114112,983040,2818048},{11,7798784,31588352,131072,1835008,1245184,1114112,3145728},{12,11403264,37879808,131072,2097152,1441792,1245184,3604480},{13,6750208,43581440,131072,1507328,655360,917504,2621440},{14,23265280,48889856,131072,2031616,1441792,1245184,3538944},{15,12320768,49479680,131072,1441792,983040,851968,2490368},{16,29425664,53477376,131072,1703936,1179648,983040,2883584},{17,27525120,56754176,131072,1572864,1114112,983040,2752512},{100,18677760,10485760,1120666,1835008,1120666,0,1835008},{101,30932992,13434880,1120666,1835008,1120666,0,1835008},{102,15859712,28639232,1238630,1966080,1238630,0,1966080},{103,12713984,46596096,1120666,1966080,1120666,0,1966080}});
            probe.set_club(club);probe.set_aim_raw(2322);probe.meter_click_raw(0);probe.meter_click_raw(power);probe.meter_click_raw(63);
            for(int tick=0;tick<700;++tick) {
                const auto before=probe.hole().ball_state();probe.step_tick();const auto after=probe.hole().ball_state();
                const auto contact=probe.hole().presentation_contact();
                if(contact.tree_id>=100) {
                    std::printf("BOARD_NATIVE_REBOUND_PASS club=%d power=%d id=%u tick=%d canopy=%d in=(%lld,%lld) out=(%lld,%lld) z=%lld dot=%g\n",club,power,contact.tree_id,tick,contact.canopy,(long long)before.vx_raw,(long long)before.vy_raw,(long long)after.vx_raw,(long long)after.vy_raw,(long long)after.z_raw,double(before.vx_raw)*after.vx_raw+double(before.vy_raw)*after.vy_raw);
                    assert((before.vx_raw || before.vy_raw) ? (double(before.vx_raw)*after.vx_raw+double(before.vy_raw)*after.vy_raw<0) : after.vz_raw>0);
                    board_seen=true;break;
                }
                if(probe.hole().phase()!=sensigolf::HoleSessionPhase::ShotActive)break;
            }
        }
        assert(board_seen);
        auto session=preview();session.register_course(1,real);session.load_next_hole();
        const auto arrows=session.verified_native_green_arrows(16);
        assert(!arrows.empty());
        session.set_original_player_mode_5a(0);
        session.set_verified_flag_object(true);
        session.set_flag_removed_for_putting(true);
        assert(session.hole().flag_removed_for_putting());
        auto flag_copy=preview();flag_copy.register_course(1,real);
        flag_copy.restore_native_session(session.serialize_native_session());
        assert(flag_copy.native_fingerprint()==session.native_fingerprint());
        session.set_flag_removed_for_putting(false);
        assert(!session.hole().flag_removed_for_putting());
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
