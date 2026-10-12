#if defined(NDEBUG)
#undef NDEBUG
#endif
#include <cassert>
#include <cstdint>
#include <limits>
#include "original_cpu_trial_force.hpp"
int main() {
    using pixtee::original_cpu_trial_force;
    auto r=original_cpu_trial_force({12,200,300,20});
    assert(r.vertical_force==440 && r.horizontal_force==540);
    r=original_cpu_trial_force({0,-20,30,0xffff});
    assert(r.vertical_force==-20 && r.horizontal_force==30);
    r=original_cpu_trial_force({
      std::numeric_limits<std::int32_t>::max(),
      std::numeric_limits<std::int32_t>::max(),
      std::numeric_limits<std::int32_t>::min(),0xffff});
    // 0x7fffffff * 0xffff mod 2^32 = 0x7fff0001
    assert(r.vertical_force==-65536);
    assert(r.horizontal_force==-65535);
    r=original_cpu_trial_force({-1,0,0,0xffff});
    assert(r.vertical_force==-65535 && r.horizontal_force==-65535);
    return 0;
}
