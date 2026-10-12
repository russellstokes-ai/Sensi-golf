#if defined(NDEBUG)
#undef NDEBUG
#endif
#include <cassert>
#include <stdexcept>
#include <vector>
#include "original_cpu_score_scratch.hpp"
int main() {
    std::vector<std::uint8_t> scratch(14*110,0xa5);
    auto r=pixtee::original_cpu_clear_scoring_records(100,20,scratch);
    assert(r.cleared_records==6 && r.reset_swing_adjuster==0);
    for(unsigned i=0;i<14*110;++i)
        assert(scratch[i]==(i<14*6?0u:0xa5u));
    bool blocked=false;
    try{pixtee::original_cpu_clear_scoring_records(500,1,scratch);}
    catch(const std::out_of_range&){blocked=true;}
    assert(blocked);
    assert(scratch[14*6]==0xa5u); // no partial mutation
    blocked=false;
    try{pixtee::original_cpu_clear_scoring_records(0,0,scratch);}
    catch(const std::invalid_argument&){blocked=true;}
    assert(blocked);
    blocked=false;
    std::vector<std::uint8_t> tiny(13,0xa5);
    try{pixtee::original_cpu_clear_scoring_records(0,1,tiny);}
    catch(const std::out_of_range&){blocked=true;}
    assert(blocked && tiny[0]==0xa5u);
    std::vector<std::uint8_t> edge(14,0xa5);
    r=pixtee::original_cpu_clear_scoring_records(65535,1,edge);
    assert(r.cleared_records==1 && edge[13]==0);
}
