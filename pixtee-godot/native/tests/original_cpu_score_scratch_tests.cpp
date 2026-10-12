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
    // Original signed two-WORD eligibility branch 0x40CA67:
    std::vector<std::uint8_t> candidate_statuses(14*110,0xff);
    auto elig=pixtee::original_cpu_first_eligible_score(
        100,20,candidate_statuses);
    assert(!elig.found && elig.original_index==5 &&
           elig.scanned_cx==100);
    const auto valid_off=14*2u;
    candidate_statuses[valid_off]=0;
    candidate_statuses[valid_off+1]=0;
    candidate_statuses[valid_off+2]=0;
    candidate_statuses[valid_off+3]=0;
    elig=pixtee::original_cpu_first_eligible_score(
        100,20,candidate_statuses);
    assert(elig.found && elig.original_index==2 &&
           elig.scanned_cx==40 && elig.scanned_bx==2 &&
           elig.scanned_dx==2);
    // A positive first status does not suffice when the second is negative.
    candidate_statuses[valid_off+2]=0xff;
    candidate_statuses[valid_off+3]=0xff;
    elig=pixtee::original_cpu_first_eligible_score(
        100,20,candidate_statuses);
    assert(!elig.found && elig.original_index==5);
    blocked=false;
    try{pixtee::original_cpu_first_eligible_score(100,0,candidate_statuses);}
    catch(const std::invalid_argument&){blocked=true;}
    assert(blocked);
    std::vector<std::uint8_t> edge(14,0xa5);
    r=pixtee::original_cpu_clear_scoring_records(65535,1,edge);
    assert(r.cleared_records==1 && edge[13]==0);
}
