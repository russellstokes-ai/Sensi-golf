#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include "sensigolf/classic_course_resources.hpp"

namespace {

std::vector<std::uint8_t> read_file(const char* path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error(std::string("cannot open ") + path);
    }
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>());
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 5) {
        std::cerr
            << "usage: classic_course_probe "
            << "<MAPMxx.MAP> <MAPMxx.SPT> <descriptor MAPI> <selector MAPI>\n";
        return 2;
    }

    try {
        sensigolf::ClassicCourseResources course(
            read_file(argv[1]),
            read_file(argv[2]),
            read_file(argv[3]),
            read_file(argv[4]));

        const auto top_left = course.lookup(0, 0, 0, 0);
        const auto tee0 = course.player_start(0);
        const auto hole = course.hole_position();

        std::cout
            << "{\"width\":" << course.map_width()
            << ",\"height\":" << course.map_height()
            << ",\"mapi_tiles\":" << course.mapi_tile_count()
            << ",\"top_left_tile\":" << course.map_tile(0, 0)
            << ",\"top_left_descriptor\":" << top_left.descriptor_index
            << ",\"top_left_slope_direction\":" << top_left.slope_direction
            << ",\"top_left_slope_magnitude\":" << top_left.slope_magnitude
            << ",\"spt_records\":5"
            << ",\"tee0_x\":" << tee0.x
            << ",\"tee0_y\":" << tee0.y
            << ",\"hole_x\":" << hole.x
            << ",\"hole_y\":" << hole.y
            << "}\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
