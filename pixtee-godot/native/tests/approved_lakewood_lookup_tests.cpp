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
    assert(argc == 2);
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
}
