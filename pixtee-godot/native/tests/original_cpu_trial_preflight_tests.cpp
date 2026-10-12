#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <limits>
#include "original_cpu_trial_preflight.hpp"
using namespace pixtee;
int main(){
    assert(original_cpu_trial_preflight({9,0,-1,100,0})
           ==OriginalCpuPreflightRoute::ReturnMinusOne);
    assert(original_cpu_trial_preflight({9,12,-1,100,0})
           ==OriginalCpuPreflightRoute::ReturnMinusOne);
    assert(original_cpu_trial_preflight({9,0,-1,100,0x140001})
           ==OriginalCpuPreflightRoute::Gravity);
    assert(original_cpu_trial_preflight({9,0,0,100,0})
           ==OriginalCpuPreflightRoute::Gravity);
    assert(original_cpu_trial_preflight({8,12,-1,0,0})
           ==OriginalCpuPreflightRoute::ReturnZero);
    assert(original_cpu_trial_preflight({8,12,-1,-1,0})
           ==OriginalCpuPreflightRoute::ReturnZero);
    assert(original_cpu_trial_preflight({8,12,-1,1,0})
           ==OriginalCpuPreflightRoute::ContinueMotion);
    assert(original_cpu_trial_preflight({8,0,-1,1,0})
           ==OriginalCpuPreflightRoute::Gravity);
    assert(original_cpu_trial_preflight({65535,12,0,
             std::numeric_limits<std::int32_t>::max(),-1})
           ==OriginalCpuPreflightRoute::ContinueMotion);
    return 0;
}
