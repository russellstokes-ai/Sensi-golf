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
    // Compose the original shot's three source-machine recovered stages,
    // never invoking gravity for original special/putter early returns.
    {
        const auto plan=original_cpu_plan_scan(4,0,512,false);
        auto result=original_cpu_trial_route_after_lookup(
            plan,31,{12,200,300,20},1,0,200,100,true);
        assert(result.trial.heading==992);
        assert(result.route==OriginalCpuPreflightRoute::Gravity);
        assert(result.gravity_contact.has_value());
        assert(result.gravity_contact->state.vertical_force==
               440-0x2100);
        result=original_cpu_trial_route_after_lookup(
            plan,0,{0,-1,3,0},9,0,0,100,true);
        assert(result.route==OriginalCpuPreflightRoute::ReturnMinusOne);
        assert(!result.gravity_contact.has_value());
        result=original_cpu_trial_route_after_lookup(
            plan,0,{0,-1,3,0},8,12,0,100,true);
        assert(result.route==OriginalCpuPreflightRoute::ContinueMotion);
        assert(!result.gravity_contact.has_value());
        result=original_cpu_trial_route_after_lookup(
            plan,0,{0,-1,0,0},8,12,0,100,true);
        assert(result.route==OriginalCpuPreflightRoute::ReturnZero);
        assert(!result.gravity_contact.has_value());
        bool invalid=false;
        try{original_cpu_trial_route_after_lookup(
            plan,0,{0,1,1,1},9,0,0,100,false);}
        catch(const std::logic_error&){invalid=true;}
        assert(invalid);
    }
    bool refused=false;
    try{original_cpu_plan_scan(10,0,0,false);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused); // Original 24-row table not available for mode10

    // Real original 0x40CCA4 trial force is now paired with a source
    // table-produced original candidate heading, not a guessed angle.
    const auto planned=original_cpu_plan_scan(4,0,512,false);
    const auto trial=original_cpu_candidate_trial(
        planned,31,{12,200,300,20});
    assert(trial.heading==992);
    assert(trial.force.vertical_force==440);
    assert(trial.force.horizontal_force==540);
    bool rejected=false;
    try{original_cpu_candidate_trial(planned,32,{12,200,300,20});}
    catch(const std::out_of_range&){rejected=true;}
    assert(rejected);
    // Two authentic original x86 CPU candidate stages compose only if
    // the preflight original course/club branch 0x40CB6C is confirmed.
    auto initial=original_cpu_plan_scan(4,0,512,false);
    const auto simulated=original_cpu_trial_precontact(
        initial,31,{12,200,300,20},0,1,true);
    assert(simulated.trial.heading==992);
    assert(simulated.trial.force.vertical_force==440);
    assert(simulated.trial.force.horizontal_force==540);
    assert(simulated.update.state.vertical_force==440-0x2100);
    assert(simulated.update.state.horizontal_force==540);
    assert(simulated.update.route==OriginalCpuContactRoute::ContinueMotion);
    bool unsupported=false;
    try{original_cpu_trial_precontact(
        initial,31,{12,200,300,20},0,1,false);}
    catch(const std::logic_error&){unsupported=true;}
    assert(unsupported);

    // Authentic evaluator record selection is source-scored and stable
    // across signed negative score values and ties. Never rank scores
    // provided without matching original candidate evaluation evidence.
    {
        auto plan=original_cpu_plan_scan(4,0,512,false);
        const auto n=plan.sweep.candidate_headings.size();
        assert(n==32);
        std::vector<std::uint8_t> records(
            n*kOriginalCpuScoreRecordBytes,0);
        const auto set_word=[&records](std::size_t index,
                                       std::size_t offset,
                                       std::uint16_t value) {
            const auto pos=index*kOriginalCpuScoreRecordBytes+offset;
            records.at(pos)=static_cast<std::uint8_t>(value);
            records.at(pos+1)=static_cast<std::uint8_t>(value>>8);
        };
        for(std::size_t i=0;i<n;++i)set_word(i,12,100u);
        set_word(5,12,0xfff8u); // original signed -8
        set_word(6,12,0xfff8u); // tie retains earliest score
        auto winner=original_cpu_choose_scored_scan(
            plan,96,3,records,true);
        assert(winner.found && winner.index==5);
        assert(winner.heading==plan.sweep.candidate_headings.at(5));
        assert(winner.score_word==0xfff8u);
        set_word(5,0,0xffffu); // first status negative: ineligible
        winner=original_cpu_choose_scored_scan(
            plan,96,3,records,true);
        assert(winner.found && winner.index==6);
        for(std::size_t i=0;i<n;++i)set_word(i,2,0xffffu);
        assert(!original_cpu_choose_scored_scan(
            plan,96,3,records,true).found);
        bool blocked=false;
        try{original_cpu_choose_scored_scan(
            plan,96,3,records,false);}
        catch(const std::logic_error&){blocked=true;}
        assert(blocked);
        blocked=false;
        try{original_cpu_choose_scored_scan(
            plan,93,3,records,true);}
        catch(const std::invalid_argument&){blocked=true;}
        assert(blocked);
        records.resize(20);
        blocked=false;
        try{original_cpu_choose_scored_scan(
            plan,96,3,records,true);}
        catch(const std::out_of_range&){blocked=true;}
        assert(blocked);
    }
    return 0;
}
