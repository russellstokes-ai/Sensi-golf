#if defined(NDEBUG)
#undef NDEBUG
#endif
#include <cassert>
#include <limits>
#include "original_cpu_simulated_motion.hpp"
using namespace pixtee;
int main() {
    auto result=original_cpu_simulated_motion(
        {0,0,0,100,0,0});
    assert(result.route==OriginalCpuMotionRoute::NextOriginalTrialTick);
    assert(result.state.x==0 && result.state.y==0);
    assert(result.state.horizontal_force==0);
    result=original_cpu_simulated_motion({1000,2000,0xF01,0,0,0});
    assert(result.state.horizontal_force==1);
    assert(result.route==OriginalCpuMotionRoute::NextOriginalTrialTick);
    result=original_cpu_simulated_motion({1000,2000,0xEFF,0,0,0});
    assert(result.state.horizontal_force==0);
    assert(result.state.x==1000 && result.state.y==2000);
    assert(result.route==OriginalCpuMotionRoute::RequiresMappedContact);
    result=original_cpu_simulated_motion({1000,2000,0xEFF,1,0,0});
    assert(result.route==OriginalCpuMotionRoute::NextOriginalTrialTick);
    assert(result.state.x==1000 && result.state.y==2000);
    result=original_cpu_simulated_motion({0,0,0x781,0,0,1});
    assert(result.state.horizontal_force==1);
    result=original_cpu_simulated_motion({0,0,0x781,0,0,2});
    assert(result.state.horizontal_force==1); // source green !=0
    result=original_cpu_simulated_motion({
       std::numeric_limits<std::int32_t>::max(),
       std::numeric_limits<std::int32_t>::min(),
       0x100000,10,1024,0});
    assert(result.route==OriginalCpuMotionRoute::NextOriginalTrialTick);
    return 0;
}
