#if defined(NDEBUG)
#undef NDEBUG
#endif
#include <cassert>
#include <stdexcept>
#include "sensigolf/recovered_animation_events.hpp"
int main(){
    using namespace sensigolf::recovered;
    OriginalAnimationSlots slots{};
    // Source reset 0x40C5C0 clears every byte except status=0xFFFF.
    OriginalAnimationSlots reset_slots{};
    for(std::size_t i=0;i<reset_slots.size();++i) {
        auto& s=reset_slots[i];
        s.status=static_cast<std::uint16_t>(i);
        s.x=100;s.y=101;s.height=102;s.width=103;s.ticks=104;
        s.asset_pointer=0xffffffffu;
        s.frame_offset=0x89abcdefu;
    }
    original_animation_reset_all(reset_slots);
    for(const auto& s:reset_slots) {
        assert(s.status==0xffffu);
        assert(s.x==0 && s.y==0 && s.height==0 &&
               s.width==0 && s.ticks==0);
        assert(s.asset_pointer==0u && s.frame_offset==0u);
    }
    OriginalAnimationAssetTable assets{};
    for(std::size_t i=0;i<assets.size();++i)
        assets[i]=static_cast<std::uint32_t>(0x420000+i*12);
    slots[0].frame_offset=0x12345678;
    auto a=original_animation_enqueue(slots,{1,2,6,12,2},assets);
    assert(a.inserted&&a.slot==0);
    assert(slots[0].status==0&&slots[0].ticks==4);
    assert(slots[0].x==1&&slots[0].y==2);
    assert(slots[0].width==6&&slots[0].height==12);
    assert(slots[0].asset_pointer==assets[2]);
    assert(slots[0].frame_offset==0x12345678);
    for(unsigned i=1;i<10;++i) {
        a=original_animation_enqueue(slots,
          {static_cast<std::uint16_t>(i),0,3,4,1},assets);
        assert(a.inserted&&a.slot==static_cast<int>(i));
    }
    a=original_animation_enqueue(slots,{1,2,6,12,2},assets);
    assert(!a.inserted && a.slot==-1);
    slots[3].status=0xffff;
    slots[6].status=0xffff;
    a=original_animation_enqueue(slots,{25,26,30,45,7},assets);
    assert(a.inserted&&a.slot==3);
    assert(slots[6].status==0xffff);
    bool refused=false;
    try{original_animation_enqueue(slots,{0,0,0,0,100},assets);}
    catch(const std::out_of_range&){refused=true;}
    assert(refused);
    OriginalAnimationSlots timed{};
    timed[0].status=0;
    timed[0].ticks=4;
    timed[0].frame_offset=0x12345678;
    timed[1].status=1;
    timed[1].ticks=1;
    auto tick=original_animation_tick_countdowns(timed);
    assert(tick.active_slots==2);
    assert(tick.newly_ready_for_script==1);
    assert(timed[0].ticks==3);
    assert(timed[1].ticks==0);
    assert(timed[0].frame_offset==0x12345678);
    bool no_script=false;
    try{original_animation_tick_countdowns(timed);}
    catch(const std::logic_error&){no_script=true;}
    assert(no_script);
    // A source-consistent host may close or decode slot 1 before
    // advancing the next animation frame.
    timed[1].status=0xffff;
    for(int i=0;i<3;++i) {
        tick=original_animation_tick_countdowns(timed);
    }
    assert(timed[0].ticks==0);
    assert(tick.newly_ready_for_script==1);
    // Original source 0x40C539 plain frame decoder. Synthetic original
    // asset bytes, never copied licensed data: delay at 0..1, actual
    // frame WORD at 6+offset. Slot+6 is repurposed by original source.
    OriginalAnimatedEvent script_slot{};
    script_slot.status=0;
    script_slot.ticks=0;
    script_slot.x=10;
    script_slot.y=20;
    script_slot.height=55;
    script_slot.width=66;
    script_slot.frame_offset=2;
    const std::vector<std::uint8_t> script{
        9,0,0,0,0,0,1,0,7,0,22,0
    };
    auto decoded=original_animation_decode_plain_frame(script_slot,script);
    assert(decoded.delay==9 && decoded.displayed_frame==7);
    assert(script_slot.ticks==9 && script_slot.frame_offset==4);
    assert(script_slot.height==7 && script_slot.width==66);
    assert(script_slot.x==10 && script_slot.y==20);
    script_slot.ticks=0;
    const auto next=original_animation_decode_plain_frame(script_slot,script);
    assert(next.displayed_frame==22);
    assert(script_slot.frame_offset==6);
    script_slot.ticks=0;
    bool no_more_frames=false;
    try{original_animation_decode_plain_frame(script_slot,script);}
    catch(const std::out_of_range&){no_more_frames=true;}
    assert(no_more_frames);
    const std::vector<std::uint8_t> script_command{
        1,0,0,0,0,0,0xff,0xff
    };
    script_slot.frame_offset=0;
    script_slot.ticks=0;
    bool not_guessed=false;
    try{original_animation_decode_plain_frame(script_slot,script_command);}
    catch(const std::logic_error&){not_guessed=true;}
    assert(not_guessed);
    assert(script_slot.frame_offset==0 && script_slot.ticks==0);
    // Multi-slot source dispatcher: a zero-tick slot reads its next
    // plain frame while a 1-tick slot decrements to zero without decoding
    // until NEXT invocation; disabled slots remain byte-exact untouched.
    OriginalAnimationSlots scripted{};
    scripted[0].status=0;
    scripted[0].ticks=0;
    scripted[0].asset_pointer=0x420000u;
    scripted[0].height=45;
    scripted[1].status=0;
    scripted[1].ticks=1;
    scripted[1].asset_pointer=0x420020u;
    const std::vector<OriginalAnimationScriptAsset> scripts{
        {0x420000u,{4,0,0,0,0,0,7,0,8,0}},
        {0x420020u,{5,0,0,0,0,0,9,0,10,0}}
    };
    auto started=original_animation_tick_with_plain_scripts(
        scripted,scripts);
    assert(started.active_slots==2);
    assert(started.newly_ready_for_script==1);
    assert(scripted[0].ticks==4 && scripted[0].height==7);
    assert(scripted[0].frame_offset==2);
    assert(scripted[1].ticks==0 && scripted[1].frame_offset==0);
    started=original_animation_tick_with_plain_scripts(scripted,scripts);
    assert(started.active_slots==2);
    assert(started.newly_ready_for_script==0);
    assert(scripted[0].ticks==3 && scripted[0].height==7);
    assert(scripted[1].ticks==5 && scripted[1].height==9);
    assert(scripted[1].frame_offset==2);
    bool missing=false;
    scripted[0].ticks=0;
    try{original_animation_tick_with_plain_scripts(scripted,{});}
    catch(const std::logic_error&){missing=true;}
    assert(missing);
    assert(scripted[0].ticks==0 && scripted[1].ticks==5);
    // Original 0x8004 frame-command uses table entry 0x40C573 to
    // release the event slot. It does not fake a callback/graphic.
    OriginalAnimatedEvent closing{};
    closing.status=0;
    closing.ticks=0;
    closing.x=20;closing.y=21;
    closing.width=15;closing.height=30;
    closing.asset_pointer=0x440001;
    closing.frame_offset=0;
    const std::vector<std::uint8_t> close_script{
        4,0,0,0,0,0,4,0x80
    };
    auto closed=original_animation_decode_supported_step(closing,close_script);
    assert(closed.kind==OriginalAnimationSupportedCommand::CloseSlot);
    assert(closing.status==0xffffu && closing.asset_pointer==0);
    assert(closing.ticks==0 && closing.frame_offset==0);
    assert(closing.x==20 && closing.y==21);
    assert(closing.width==15 && closing.height==30);
    OriginalAnimatedEvent waiting{};
    waiting.status=0;waiting.ticks=0;waiting.frame_offset=0;
    waiting.height=88;
    const std::vector<std::uint8_t> wait_script{
        4,0,0,0,0,0,8,0x80,17,0
    };
    auto waited=original_animation_decode_supported_step(waiting,wait_script);
    assert(waited.kind==OriginalAnimationSupportedCommand::WaitTicks);
    assert(waited.delay==17 && waiting.ticks==17);
    assert(waiting.frame_offset==4 && waiting.height==88);
    // The true original 0x8000 callback is NOT inferred, and may
    // execute arbitrary animation/sound code: reject without edits.
    OriginalAnimatedEvent callback{};
    callback.status=0;callback.asset_pointer=123;
    callback.frame_offset=0;callback.ticks=0;
    bool callback_refused=false;
    try{original_animation_decode_supported_step(
        callback,{4,0,0,0,0,0,0,0x80});}
    catch(const std::logic_error&){callback_refused=true;}
    assert(callback_refused && callback.status==0);
    assert(callback.frame_offset==0 && callback.asset_pointer==123);





    // Original 0x8000 command is exposed ONLY as a safe host handoff:
    // no untrusted callback machine pointer is ever executed by C++.
    OriginalAnimatedEvent callback_handoff{};
    callback_handoff.status=0;
    callback_handoff.ticks=0;
    callback_handoff.asset_pointer=0x501000u;
    callback_handoff.frame_offset=2;
    callback_handoff.height=54;
    const std::vector<std::uint8_t> callback_script{
        7,0,0,0,0,0,0,0,
        0,0x80,0x00,0x02,0x50,0x00
    };
    const auto requested=original_animation_prepare_callback(
        callback_handoff,callback_script);
    assert(requested.source_callback_token==0x00500200u);
    assert(requested.original_asset_pointer==0x501000u);
    assert(requested.original_frame_offset==2);
    assert(requested.script_delay==7);
    assert(callback_handoff.ticks==0 && callback_handoff.frame_offset==2);
    original_animation_finalize_callback(callback_handoff,requested);
    assert(callback_handoff.ticks==7);
    assert(callback_handoff.frame_offset==8);
    assert(callback_handoff.height==54 && callback_handoff.status==0);
    bool stale_ack=false;
    try{original_animation_finalize_callback(callback_handoff,requested);}
    catch(const std::logic_error&){stale_ack=true;}
    assert(stale_ack);
    OriginalAnimationSlots blocked_callback{};
    blocked_callback[0].status=0;
    blocked_callback[0].ticks=0;
    blocked_callback[0].frame_offset=2;
    blocked_callback[0].asset_pointer=0x501000u;
    bool refused_direct_call=false;
    try{original_animation_tick_with_plain_scripts(blocked_callback,
        {{0x501000u,callback_script}});}
    catch(const std::logic_error&){refused_direct_call=true;}
    assert(refused_direct_call);
    assert(blocked_callback[0].ticks==0 &&
           blocked_callback[0].frame_offset==2);

}
