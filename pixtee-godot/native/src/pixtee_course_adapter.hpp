#pragma once
#include <cstdint>
#include "sensigolf/classic_course_resources.hpp"
namespace pixtee {
// Original-format 8x4 fine MAPI collision lookup and separate MAPS green
// map, with Pixtee-authored coordinates; C++ original physics is untouched.
sensigolf::ClassicCourseResources make_lakewood_hole_1();
sensigolf::ClassicCourseResources make_lakewood_hole_1_with_slope(
    std::uint8_t direction, std::uint8_t magnitude);
}
