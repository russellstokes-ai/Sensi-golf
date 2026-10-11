#include "original_next_player.hpp"
#include <stdexcept>
namespace pixtee {
std::uint8_t original_next_player_index(
    const OriginalPlayerChoiceInput& in) {
    if(in.count==0 || in.count>4)
        throw std::invalid_argument("Windows v1.014 player count 1..4");
    if(in.forced_holder) {
        // 0x40C1C0..0x40C1E3: loop until a player identity equals
        // (player count - forced holder), byte arithmetic, no fallback.
        const std::uint8_t target=static_cast<std::uint8_t>(
            in.count-in.forced_holder);
        for(std::uint8_t i=0;i<in.count;++i)
            if(in.identity[i]==target) return i;
        throw std::logic_error("original forced turn identity not found");
    }
    // 0x40C1F1..0x40C22D: the FIRST slot is seeded without applying
    // the elimination/zero rules; later slots require nonzero priority and
    // player[0x5e] != 1. Equal priority favors the LAST eligible slot.
    std::uint8_t selected=0;
    std::uint32_t best=in.priority_metric[0];
    for(std::uint8_t i=1;i<in.count;++i) {
        const auto score=in.priority_metric[i];
        if(score==0 || in.eliminated[i]==1 || best>score)continue;
        best=score;
        selected=i;
    }
    return selected;
}
}
