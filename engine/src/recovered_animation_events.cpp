#include "sensigolf/recovered_animation_events.hpp"
#include <stdexcept>
#include <algorithm>
namespace sensigolf::recovered {
void original_animation_reset_all(OriginalAnimationSlots& slots) noexcept {
    // Source v1.014 0x40C5C0: STOSW(0xffff), then 9 STOSW(0)
    // repeated for 10 packed event records of 20 bytes each.
    // C++ assigns field values, not potentially padded raw bytes.
    for(auto& slot:slots)slot=OriginalAnimatedEvent{};
}
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
        if((frame&0x8000u)!=0) {
            if(frame!=0x8004u && frame!=0x8008u)
                throw std::logic_error(
                    "original animation script callback/opcode not recovered");
            if(frame==0x8008u && bytes.size()-offset<10)
                throw std::out_of_range(
                    "original animation wait command payload truncated");
        }
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
        (void)original_animation_decode_supported_step(slot,it->script);
    }
    return result;
}

OriginalAnimationSupportedStep original_animation_decode_supported_step(
    OriginalAnimatedEvent& slot,const std::vector<std::uint8_t>& script) {
    if(slot.status==0xffffu || slot.ticks!=0)
        throw std::logic_error(
            "original animation decoder requires active slot and zero tick");
    const auto offset=static_cast<std::size_t>(slot.frame_offset);
    if(script.size()<2 || offset>script.size() ||
       script.size()-offset<8)
        throw std::out_of_range(
            "original animation script missing frame/command word");
    const auto opcode=static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(script[6u+offset])|
        (static_cast<std::uint16_t>(script[7u+offset])<<8));
    if((opcode&0x8000u)==0) {
        const auto result=original_animation_decode_plain_frame(slot,script);
        return {OriginalAnimationSupportedCommand::PlainFrame,
                result.displayed_frame,result.delay};
    }
    // Refuse unverified original indirect callbacks/opcodes before
    // mutating any slot state. Only recovered 0x8004/0x8008 are legal.
    if(opcode!=0x8004u && opcode!=0x8008u)
        throw std::logic_error(
            "original animation script command not source-authenticated");
    if(opcode==0x8004u) {
        // Source 0x40C539 -> 0x40C55A -> 0x40C573.
        // Delay assignment preceding the indirect JMP is overwritten
        // with zero by the original release handler.
        slot.asset_pointer=0;
        slot.frame_offset=0;
        slot.ticks=0;
        slot.status=0xffffu;
        return {OriginalAnimationSupportedCommand::CloseSlot,0,0};
    }
    // Source 0x40C58D reads the WORD after command and adds a second
    // 2 bytes to the frame offset (first +2 was 0x40C549).
    if(script.size()-offset<10)
        throw std::out_of_range(
            "source original 0x8008 wait command missing tick WORD");
    const auto ticks=static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(script[8u+offset])|
        (static_cast<std::uint16_t>(script[9u+offset])<<8));
    slot.ticks=ticks;
    slot.frame_offset=static_cast<std::uint32_t>(slot.frame_offset+4u);
    return {OriginalAnimationSupportedCommand::WaitTicks,0,ticks};
}


OriginalAnimationCallbackRequest original_animation_prepare_callback(
    const OriginalAnimatedEvent& slot,
    const std::vector<std::uint8_t>& script) {
    if(slot.status==0xffffu || slot.ticks!=0)
        throw std::logic_error(
            "original source callback only accepted for active zero-tick slot");
    const auto offset=static_cast<std::size_t>(slot.frame_offset);
    if(offset>script.size() || script.size()-offset<12u)
        throw std::out_of_range("original callback command/pointer truncated");
    const auto word16=[&](std::size_t i) {
        return static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(script[i])|
            (static_cast<std::uint16_t>(script[i+1])<<8));
    };
    if(word16(offset+6u)!=0x8000u)
        throw std::logic_error("next original script opcode is not 0x8000");
    const auto ptr=static_cast<std::uint32_t>(
        static_cast<std::uint32_t>(script[offset+8u])|
        (static_cast<std::uint32_t>(script[offset+9u])<<8)|
        (static_cast<std::uint32_t>(script[offset+10u])<<16)|
        (static_cast<std::uint32_t>(script[offset+11u])<<24));
    return {slot.asset_pointer,ptr,slot.frame_offset,word16(0u)};
}

void original_animation_finalize_callback(
    OriginalAnimatedEvent& slot,
    const OriginalAnimationCallbackRequest& ack) {
    if(slot.status==0xffffu || slot.ticks!=0 ||
       slot.asset_pointer!=ack.original_asset_pointer ||
       slot.frame_offset!=ack.original_frame_offset)
        throw std::logic_error(
            "stale or invalid original host callback acknowledgement");
    // Source 0x40C539..0x40C56D first advances offset by WORD(2),
    // calls original supplied function pointer, and then adds DWORD(4).
    // NEVER call the pointer directly. This helper is invoked only
    // after an independently authored host callback has completed.
    slot.ticks=ack.script_delay;
    slot.frame_offset=static_cast<std::uint32_t>(
        slot.frame_offset+static_cast<std::uint32_t>(6u));
}

} // namespace sensigolf::recovered
