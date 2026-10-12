#include "sensigolf/recovered_flag_setup.hpp"
namespace sensigolf::recovered {
OriginalFlagSetup original_flag_setup_from_spt(
    const ClassicCourseResources& course) {
    const auto r=course.spt_record(4);
    return {r.words[2],r.words[3],3};
}
} // namespace sensigolf::recovered
