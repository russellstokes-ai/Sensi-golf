#if defined(NDEBUG)
#undef NDEBUG
#endif
#include <cassert>
#include <stdexcept>
#include "original_cpu_scan_pipeline.hpp"
using namespace pixtee;
int main() {
    for(std::uint16_t mode=4;mode<=9;++mode) {
        for(std::uint8_t band=0;band<4;++band) {
            auto plan=original_cpu_plan_scan(mode,band,2048,false);
            const auto size=plan.sweep.candidate_headings.size();
            assert(size>0 && size<=110);
            std::vector<OriginalComputerCandidate> sampled(size);
            for(auto& c:sampled) {
                c.status_primary=-1;
                c.status_secondary=-1;
            }
            auto retry=original_cpu_rank_scanned_candidates(plan,sampled);
            assert(retry.requires_original_retry);
            bool failed=false;
            try{
                original_cpu_resolve_sampled_swing(retry,false,80,1,40,8,15);
            }catch(const std::logic_error&){failed=true;}
            assert(failed);
            sampled[0]={0,0,60};
            sampled[size-1]={0,0,10};
            auto sel=original_cpu_rank_scanned_candidates(plan,sampled);
            assert(!sel.requires_original_retry);
            assert(sel.original_candidate_index==static_cast<int>(size-1));
            assert(sel.original_candidate_heading==
                   plan.sweep.candidate_headings[size-1]);
            const auto swing=original_cpu_resolve_sampled_swing(
                sel,false,80,plan.difficulty.variation_budget,40,8,15);
            assert(swing.ranked.original_candidate_index==sel.original_candidate_index);
            assert(swing.variation.captured_power<=65535);
            bool tooFew=false;
            sampled.pop_back();
            try{original_cpu_rank_scanned_candidates(plan,sampled);}
            catch(const std::invalid_argument&){tooFew=true;}
            assert(tooFew);
        }
    }
    bool refused=false;
    try{original_cpu_plan_scan(10,0,0,false);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused); // Original 24-row table not available for mode10
    return 0;
}
