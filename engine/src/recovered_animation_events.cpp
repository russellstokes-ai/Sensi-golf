#include "sensigolf/recovered_animation_events.hpp"
#include <stdexcept>
#include <algorithm>
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

OriginalAnimationPlainFrame original_animation_decode_plain_frame(
    OriginalAnimatedEvent& slot,
    const std::vector<std::uint8_t>& script) {
    if(slot.status==0xffffu || slot.ticks!=0)
        throw std::logic_error(
            "original frame decoder requires an active event with zero countdown");
    // Original EDI = [slot+0xC]+6+[slot+0x10], AX=WORD[EDI].
    // Test only within the caller-provided original script byte range.
    const auto offset=static_cast<std::size_t>(slot.frame_offset);
    if(script.size()<2 || offset>script.size() ||
       script.size()-offset<8u)
        throw std::out_of_range(
            "original animation script has no complete frame at offset");
    const std::uint16_t frame=static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(script[6u+offset]) |
        (static_cast<std::uint16_t>(script[7u+offset])<<8));
    if((frame&0x8000u)!=0)
        throw std::logic_error(
            "original negative animation script opcode decoder remains unverified");
    const std::uint16_t delay=static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(script[0]) |
        (static_cast<std::uint16_t>(script[1])<<8));
    slot.ticks=delay;
    slot.frame_offset=static_cast<std::uint32_t>(slot.frame_offset+2u);
    slot.height=frame;
    return {frame,delay};
}

OriginalAnimationTimerTick original_animation_tick_with_plain_scripts(
    OriginalAnimationSlots& slots,
    const std::vector<OriginalAnimationScriptAsset>& scripts) {
    // Fail *before* mutating the other original slots if source asset
    // bytes are missing or the next source WORD is a negative command
    // opcode. Does not pretend to emulate the unseen opcode interpreter.
    for(const auto& slot:slots) {
        if(slot.status==0xffffu || slot.ticks!=0)continue;
        const auto it=std::find_if(scripts.begin(),scripts.end(),
            [&slot](const OriginalAnimationScriptAsset& asset) {
                return asset.source_pointer==slot.asset_pointer;
            });
        if(it==scripts.end())
            throw std::logic_error(
                "original animation pending frame has no mapped source script");
        const auto offset=static_cast<std::size_t>(slot.frame_offset);
        const auto& bytes=it->script;
        if(bytes.size()<2 || offset>bytes.size() ||
           bytes.size()-offset<8u)
            throw std::out_of_range(
                "original animation script frame not mapped");
        const auto frame=static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(bytes[6u+offset])|
            (static_cast<std::uint16_t>(bytes[7u+offset])<<8));
        if((frame&0x8000u)!=0)
            throw std::logic_error(
                "original animation script command opcode not decoded");
    }
    OriginalAnimationTimerTick result{};
    for(auto& slot:slots) {
        if(slot.status==0xffffu)continue;
        ++result.active_slots;
        if(slot.ticks!=0) {
            --slot.ticks;
            if(slot.ticks==0)++result.newly_ready_for_script;
            continue;
        }
        const auto it=std::find_if(scripts.begin(),scripts.end(),
            [&slot](const OriginalAnimationScriptAsset& asset) {
                return asset.source_pointer==slot.asset_pointer;
            });
        (void)original_animation_decode_plain_frame(slot,it->script);
    }
    return result;
}

} // namespace sensigolf::recovered
