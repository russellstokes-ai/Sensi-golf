#include "sensigolf/recovered_animation_events.hpp"
#include <stdexcept>
namespace sensigolf::recovered {
OriginalAnimationEnqueueResult original_animation_enqueue(
    OriginalAnimationSlots& slots,
    OriginalAnimationEventInput event,
    const OriginalAnimationAssetTable& assets) {
    // The source BP lookup is only source-authenticated for valid
    // 0..15 resource-table indices. Original machine does not bounds
    // check; reject invalid host IDs rather than index unrelated memory.
    if(event.asset_index>=assets.size())
        throw std::out_of_range(
            "original source animation asset index outside authenticated table");
    for(std::size_t i=0;i<slots.size();++i) {
        auto& slot=slots[i];
        if(slot.status!=0xffffu)continue;
        slot.status=0;
        slot.ticks=4;
        slot.x=event.x;slot.y=event.y;
        slot.height=event.height;slot.width=event.width;
        slot.asset_pointer=assets[event.asset_index];
        // Authentic source does NOT zero previous frame_offset at +0x10
        // when the next event reuses a slot.
        return {static_cast<int>(i),true};
    }
    return {-1,false};
}
OriginalAnimationTimerTick original_animation_tick_countdowns(
    OriginalAnimationSlots& slots) {
    for(const auto& slot:slots) {
        if(slot.status!=0xffffu && slot.ticks==0)
            throw std::logic_error(
                "original 0x40C539 event script decoder not recovered for this slot");
    }
    OriginalAnimationTimerTick result{};
    for(auto& slot:slots) {
        if(slot.status==0xffffu)continue;
        ++result.active_slots;
        --slot.ticks;
        if(slot.ticks==0)++result.newly_ready_for_script;
    }
    return result;
}

} // namespace sensigolf::recovered
