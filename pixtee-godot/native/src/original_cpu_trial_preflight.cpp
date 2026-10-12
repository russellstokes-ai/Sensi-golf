#include "original_cpu_trial_preflight.hpp"
namespace pixtee {
OriginalCpuPreflightRoute original_cpu_trial_preflight(
    const OriginalCpuPreflightState& s) noexcept {
    // Original signed JNS / JG / JE at 0x40CB40..0x40CB53:
    // descending or negative vertical force and low height on
    // landing code 9 terminate the simulated candidate.
    if(s.vertical_force<0 && s.height<=0x140000 &&
       s.terrain_code==9u)
        return OriginalCpuPreflightRoute::ReturnMinusOne;
    // Original original 0x40CB59 only club index 12 is the putter.
    // For every other club, even code9 when ascending, gravity runs.
    if(s.club!=12u) return OriginalCpuPreflightRoute::Gravity;
    // Source 0x40CB60 signed JLE is a terminal return without motion;
    // positive putter horizontal force enters normal motion.
    return s.horizontal_force>0
        ? OriginalCpuPreflightRoute::ContinueMotion
        : OriginalCpuPreflightRoute::ReturnZero;
}
} // namespace pixtee
