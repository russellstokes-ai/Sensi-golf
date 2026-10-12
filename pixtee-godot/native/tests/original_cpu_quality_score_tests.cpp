#if defined(NDEBUG)
#undef NDEBUG
#endif
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include "original_cpu_quality_score.hpp"
int main(){
    pixtee::OriginalCpuPenaltyTable table{};
    table[0]=0;
    table[8]=0x9000u;
    table[9]=0xffffu;
    table[31]=65535;
    assert(pixtee::original_cpu_quality_score(100,8,0,table)==0x9064u);
    assert(pixtee::original_cpu_quality_score(100,8,1,table)==100u);
    assert(pixtee::original_cpu_quality_score(100,8,2,table)==0x9064u);
    assert(pixtee::original_cpu_quality_score(1,9,0,table)==0u);
    assert(pixtee::original_cpu_quality_score(2,31,0,table)==1u);
    const auto& source=pixtee::original_cpu_reference_penalties();
    assert(source[0]==200 && source[1]==40 && source[4]==5);
    assert(source[10]==1000 && source[18]==200);
    assert(source[31]==0);
    assert(pixtee::original_cpu_quality_score(50,0,0,source)==250u);
    assert(pixtee::original_cpu_quality_score(50,10,0,source)==1050u);
    assert(pixtee::original_cpu_quality_score(50,10,1,source)==50u);
    assert(pixtee::original_cpu_quality_score(65000,0,0,source)==65200u);
    bool rejected=false;
    try{pixtee::original_cpu_quality_score(0,0xffffu,0,table);}
    catch(const std::out_of_range&){rejected=true;}
    assert(rejected);
    rejected=false;
    try{pixtee::original_cpu_quality_score(0,32u,0,table);}
    catch(const std::out_of_range&){rejected=true;}
    assert(rejected);
    return 0;
}
