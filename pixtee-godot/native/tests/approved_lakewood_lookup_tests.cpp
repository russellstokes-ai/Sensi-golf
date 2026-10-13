#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include "sensigolf/classic_course_resources.hpp"
#include "sensigolf/classic_hole_session.hpp"

std::vector<std::uint8_t> read(const std::string& root, const char* file) {
    std::ifstream input(root + "/" + file, std::ios::binary);
    assert(input.good());
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}
int main(int argc, char** argv) {
    assert(argc == 3);
    const std::string root = argv[1];
    sensigolf::ClassicCourseResources course(read(root,"MAPM.bin"),
        read(root,"SPT.bin"), read(root,"MAPI-descriptors.bin"),
        read(root,"MAPI-selectors.bin"));
    assert(course.map_width()==34 && course.map_height()==114);
    assert(course.player_start(0).x==256 && course.player_start(0).y==850);
    assert(course.hole_position().x==364 && course.hole_position().y==168);
    const auto expected = read(root,"expected-subcells.bin");
    assert(expected.size()==272*456);
    std::size_t count=0;
    for (int y=0; y<912; y+=2) for (int x=0; x<544; x+=2) {
        const auto target=expected[count++];
        // Exhaustively checks native decomposition, selector bit order,
        // row orientation and descriptor lookup, including every tile seam.
        for(int dy=0;dy<2;++dy) for(int dx=0;dx<2;++dx)
            assert(course.resolve_integer_position(x+dx,y+dy).descriptor_index==target);
    }
    assert(course.resolve_integer_position(256,850).landing_code==3);
    assert(course.resolve_integer_position(364,168).landing_code==1);
    assert(course.resolve_integer_position(460,510).landing_code==35);
    assert(course.resolve_integer_position(250,480).landing_code==7);
    // Run a genuine complete source-core shot on approved coordinates.
    sensigolf::ClassicHoleSession hole(course,std::size_t{0},
        sensigolf::ClassicHoleMetadata{0,4,0});
    sensigolf::ClassicShotRequest request{};
    request.club_index=0; request.aim_raw=2048;
    request.power_tick=105; request.accuracy_tick=63;
    hole.begin_shot(request);
    int ticks=0, contacts=0;
    while(hole.phase()==sensigolf::HoleSessionPhase::ShotActive && ticks++<4096) {
        hole.step();
        if(hole.presentation_contact().impact) ++contacts;
    }
    assert(contacts>0 && contacts<32);
    assert(ticks>0 && ticks<4096);
    assert(hole.strokes()>=1);
    assert(hole.phase()!=sensigolf::HoleSessionPhase::UnsupportedTerrain);
    std::cout << "approved Lakewood: 496128 native pixel lookups; shot ticks="
              << ticks << " phase=" << static_cast<int>(hole.phase()) << '\n';
    const std::string green_root=argv[2];
    sensigolf::ClassicCourseResources green(read(green_root,"MAPM.bin"),read(green_root,"SPT.bin"),
        read(green_root,"MAPI-descriptors.bin"),read(green_root,"MAPI-selectors.bin"),read(green_root,"MAPS.bin"));
    const auto region=green.green_region();
    assert(region && region->origin_x==304 && region->origin_y==120);
    const auto words=read(green_root,"expected-detail-words.bin");
    assert(words.size()==128*104*2);
    for(int y=0;y<104;++y)for(int x=0;x<128;++x) {
        const auto offset=2*(y*128+x);
        const auto word=(unsigned(words[offset])<<8)|words[offset+1];
        const auto native=green.resolve_integer_position(2*x+1,2*y+1,true);
        assert(native.descriptor_index==(word&127));
        assert(native.slope_direction==((word>>8)&15)*256);
        assert(native.slope_magnitude==((word>>12)&15));
        // Adding markers and appending MAPI must preserve course lookup.
        assert(green.resolve_integer_position(304+x,120+y).descriptor_index==
               course.resolve_integer_position(304+x,120+y).descriptor_index);
    }
    sensigolf::ClassicHoleSession approach(green,320*65536,140*65536);
    request.club_index=0;request.aim_raw=2048;request.power_tick=30;
    approach.begin_shot(request);
    bool entered=false,left=false;
    for(int tick=0;tick<4096 && approach.phase()==sensigolf::HoleSessionPhase::ShotActive;++tick) {
        approach.step();
        if(approach.green_mode())entered=true;
        else if(entered)left=true;
    }
    assert(entered && left && approach.phase()==sensigolf::HoleSessionPhase::ReadyForShot);
    sensigolf::ClassicHoleSession cup(green,364*65536,168*65536,sensigolf::ClassicHoleMetadata{0,4,1});
    request.club_index=12;request.power_tick=1;request.aim_raw=0;
    cup.begin_shot(request);cup.step();
    assert(cup.green_mode() && cup.phase()==sensigolf::HoleSessionPhase::CupTerminal);
    assert(cup.ball_x_raw()==120*65536 && cup.ball_y_raw()==96*65536);
    assert(cup.distance_to_hole()==0 && cup.pending_cup_capture_event().has_value());
    cup.step();
    assert(cup.phase()==sensigolf::HoleSessionPhase::HoleScored && cup.strokes()==1);
    std::cout<<"LAKEWOOD_GREEN_NATIVE_PASS: 13312 detail cells, unchanged world map, native entry/exit and cup-to-score\n";
}
