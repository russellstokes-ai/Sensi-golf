#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdint>
#include <limits>
#include "original_cpu_simulated_contact.hpp"
using namespace pixtee;
int main(){
    auto st=original_cpu_simulation_contact({0,0xd000,0,0});
    assert(st.route==OriginalCpuContactRoute::RequestOriginalLipRandom);
    assert(st.state.vertical_force==0xd000);
    assert(st.state.horizontal_force==0x6800);
    assert(st.state.ball_height==0);
    st=original_cpu_simulation_contact({0,0xcfff,0,0});
    assert(st.route==OriginalCpuContactRoute::ReturnZeroStop);
    assert(st.state.vertical_force==-0x2100);
    st=original_cpu_simulation_contact({0,0xcfff,0,1});
    assert(st.route==OriginalCpuContactRoute::ContinueMotion);
    assert(st.state.vertical_force==-0x2100);
    st=original_cpu_simulation_contact({0,0xd000,0x10000,0});
    assert(st.route==OriginalCpuContactRoute::ContinueMotion);
    st=original_cpu_simulation_contact(
        {std::numeric_limits<std::int32_t>::max(),0xd000,
         std::numeric_limits<std::int32_t>::max(),0});
    assert(st.state.ball_height==0); // signed overflow wrap to negative
    return 0;
}
