#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include "sensigolf/recovered_mapped_terminal.hpp"

using namespace sensigolf::recovered;

int main() {
    std::array<std::uint8_t,8> desc{};
    std::array<std::uint8_t,8> mask{};
    OriginalMappedTerminalInput in{};
    in.descriptor_bank=desc.data();
    in.descriptor_size=desc.size();
    in.selector_bank=mask.data();
    in.selector_size=mask.size();
    in.vertical_force=-8448;
    in.horizontal_force=50000;
    in.player_flags=2;
    in.contact_flags=4;

    // Four source MAPI descriptor words with independent selector planes.
    desc={0,6,0,47,0,7,0,4};
    auto a=original_mapped_terminal_contact(in);
    assert(a.resolved && a.descriptor_index==6 && a.landing_code==35);
    assert(a.vertical_force==0 && a.horizontal_force==0);
    assert(a.player_flags==0x8002u && a.event_code==11);
    assert(a.contact_flags==4);
    // Select word one: original descriptor 47 is landing code 50.
    mask[0]=0x80;
    a=original_mapped_terminal_contact(in);
    assert(a.resolved && a.descriptor_index==47 && a.landing_code==50);
    assert(a.event_code==1 && a.player_flags==2 && a.contact_flags==4);
    assert(a.vertical_force==4224 && a.horizontal_force==52112);
    // Select word two: original descriptor 7 is code 8, intentionally gated.
    mask[0]=0;
    mask[4]=0x80;
    a=original_mapped_terminal_contact(in);
    assert(!a.resolved && a.descriptor_index==7 && a.landing_code==8);
    // Default word 0, code 35 above 200 enters generic event-0 latch.
    mask[4]=0;
    in.ball_height=201<<16;
    a=original_mapped_terminal_contact(in);
    assert(a.resolved && a.landing_code==35 && a.event_code==0);
    assert(a.contact_flags==5 && a.player_flags==2);
    // Repeated contact suppresses event 0 but retains bounce.
    in.contact_flags=5;
    a=original_mapped_terminal_contact(in);
    assert(a.resolved && a.event_code==-1 && a.contact_flags==5);
    // Source entry with no vertical force cannot produce terminal effects.
    in.vertical_force=0;
    a=original_mapped_terminal_contact(in);
    assert(!a.resolved && a.event_code==-1);
    in.vertical_force=1;
    bool thrown=false;
    try { in.tile_index=2; (void)original_mapped_terminal_contact(in); }
    catch(const std::out_of_range&) { thrown=true; }
    assert(thrown);
    // Original 0x40A575..0x40A5F2 pre-putter map admission.
    {
        OriginalMappedTerminalInput check{};
        std::array<std::uint8_t,8> d{0,6,0,7,0,23,0,47};
        std::array<std::uint8_t,8> m{};
        check.descriptor_bank=d.data();
        check.descriptor_size=d.size();
        check.selector_bank=m.data();
        check.selector_size=m.size();
        check.club_index=0;
        check.contact_flags=0x8001u;
        check.ball_height=20<<16;
        auto out=original_putter_terrain_admission(check,0);
        assert(out.route==OriginalPutterAdmissionRoute::NormalFlight);
        assert(!out.map_lookup_performed && out.contact_flags==0x8000u);
        check.ball_height=19<<16;
        out=original_putter_terrain_admission(check,0);
        assert(out.contact_flags==0x8001u);
        check.club_index=12;
        out=original_putter_terrain_admission(check,1);
        assert(out.route==OriginalPutterAdmissionRoute::NormalFlight);
        assert(!out.map_lookup_performed);
        out=original_putter_terrain_admission(check,2);
        assert(out.map_lookup_performed && out.landing_code==35);
        assert(out.route==OriginalPutterAdmissionRoute::Code35LowHeight);
        check.ball_height=201<<16;
        out=original_putter_terrain_admission(check,0);
        assert(out.route==OriginalPutterAdmissionRoute::HighCodeEvent1);
        m[0]=0x80;
        out=original_putter_terrain_admission(check,0);
        assert(out.route==OriginalPutterAdmissionRoute::CupCode8);
        m[0]=0; m[4]=0x80;
        out=original_putter_terrain_admission(check,0);
        assert(out.route==OriginalPutterAdmissionRoute::SpecialGreenCode9Or10);
        m[0]=0x80;
        out=original_putter_terrain_admission(check,0);
        assert(out.route==OriginalPutterAdmissionRoute::HighCodeEvent1);
        check.club_index=5;
        check.tile_index=300;
        // Original nonputter path bypasses MAPI; source-scoped function
        // must not attempt a lookup on any invalid/unmapped tile.
        out=original_putter_terrain_admission(check,0);
        assert(!out.map_lookup_performed);
    }

}
