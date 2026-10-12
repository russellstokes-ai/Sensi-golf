#pragma once
#include <cstdint>
#include "sensigolf/classic_course_resources.hpp"
namespace sensigolf::recovered {
// Original v1.014 0x409F39..0x409F64 reads fifth 10-byte SPT record's
// two big-endian position words and always writes original global flag
// marker 3. This proves position/marker SETUP, not presence of every
// physical flagpole sprite on a new authored Pixtee course.
struct OriginalFlagSetup {
    std::uint16_t x=0;
    std::uint16_t y=0;
    std::uint16_t source_marker=3;
};
OriginalFlagSetup original_flag_setup_from_spt(
    const ClassicCourseResources& course);
} // namespace sensigolf::recovered
