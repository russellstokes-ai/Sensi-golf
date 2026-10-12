#if defined(NDEBUG)
#undef NDEBUG
#endif
#include <cassert>
#include <stdexcept>
#include "sensigolf/recovered_animation_events.hpp"
int main(){
    using namespace sensigolf::recovered;
    OriginalAnimationSlots slots{};
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


}
