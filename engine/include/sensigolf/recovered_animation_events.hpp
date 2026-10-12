#pragma once
#include <array>
#include <cstdint>
namespace sensigolf::recovered {
// Original Windows v1.014 animation event slots 0x41F8F0, each 0x14 bytes.
// Source 0x40C4C0..0x40C50A enqueues into FIRST status==0xFFFF slot.
// The animation frame stepping at 0x40C50B.. is a SEPARATE source stage.
struct OriginalAnimatedEvent {
    std::uint16_t status=0xffff;
    std::uint16_t x=0;
    std::uint16_t y=0;
    std::uint16_t height=0; // original [+6] <- DX
    std::uint16_t width=0;  // original [+8] <- CX
    std::uint16_t ticks=0;  // original [+A] <- 4
    std::uint32_t asset_pointer=0;// original [+C] from BP lookup
    std::uint32_t frame_offset=0;// original [+10] UNCHANGED on enqueue
};
struct OriginalAnimationEventInput {
    std::uint16_t x=0;
    std::uint16_t y=0;
    std::uint16_t width=0;
    std::uint16_t height=0;
    std::uint16_t asset_index=0;
};
constexpr std::size_t kOriginalAnimationEventSlots=10u;
using OriginalAnimationSlots=
    std::array<OriginalAnimatedEvent,kOriginalAnimationEventSlots>;
using OriginalAnimationAssetTable=std::array<std::uint32_t,16>;
struct OriginalAnimationEnqueueResult {
    int slot=-1; // -1 means original RET without altering any slot
    bool inserted=false;
};
OriginalAnimationEnqueueResult original_animation_enqueue(
    OriginalAnimationSlots& slots,
    OriginalAnimationEventInput event,
    const OriginalAnimationAssetTable& source_asset_table);
} // namespace sensigolf::recovered
