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
}
