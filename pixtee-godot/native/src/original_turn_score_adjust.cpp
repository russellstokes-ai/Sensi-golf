#include "original_turn_score_adjust.hpp"
#include <stdexcept>
namespace pixtee {
OriginalTurnScoreAdjustment original_turn_score_adjust(
    const OriginalTurnScores& source) {
    if(source.count<1 || source.count>4)
        throw std::invalid_argument("Original player count must be 1..4");
    OriginalTurnScoreAdjustment result{};
    result.result=source;
    // Original 0x40C022: only players with ball+0x18 == 0 considered.
    for(std::uint8_t i=0;i<source.count;++i){
        if(source.ball_distance[i]!=0)continue;
        const auto score=static_cast<std::int16_t>(source.strokes[i]);
        bool minimum=true;
        for(std::uint8_t j=0;j<source.count;++j){
            if(i!=j && score>static_cast<std::int16_t>(source.strokes[j])){
                minimum=false;
                break;
            }
        }
        if(!minimum)continue;
        // 0x40C065..0x40C074: a second eligible marked player with
        // equal per-hole strokes aborts the entire source mode-1 branch.
        for(std::uint8_t j=0;j<source.count;++j){
            if(i!=j && source.completed_marker[j]!=0 &&
               source.strokes[j]==source.strokes[i])
                return result;
        }
        // 0x40C07F/0x40C083: *both* 16-bit counters decremented.
        // Exact x86 word wrap is required.
        result.result.strokes[i]=static_cast<std::uint16_t>(
            result.result.strokes[i]-1u);
        result.result.total_strokes[i]=static_cast<std::uint16_t>(
            result.result.total_strokes[i]-1u);
        result.corrected_index=i;
        return result; // original jumps to 0x40BEF4
    }
    return result;
}
}
