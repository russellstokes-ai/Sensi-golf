#pragma once
#include <array>
#include <cstdint>
#include <vector>
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
// Original 0x40C50B..0x40C539 round-robin timer-only stage.
// Precise source precondition: every active slot must have ticks>0,
// otherwise the source must decode its animation SCRIPT at 0x40C539.
// Do not silently invent event animation frames.
struct OriginalAnimationTimerTick {
    std::size_t active_slots=0;
    std::size_t newly_ready_for_script=0; // countdown 1->0 this tick
};
OriginalAnimationTimerTick original_animation_tick_countdowns(
    OriginalAnimationSlots& slots);

// Original Windows v1.014 0x40C539..0x40C559. This is strictly the
// nonnegative-frame branch; negative script words are dynamic source
// opcodes and have NOT been recovered. The original frame word at
// [slot+6] replaces the queue's previous height field as the
// animation advances (a reused structure union in original code).
struct OriginalAnimationPlainFrame {
    std::uint16_t displayed_frame=0;
    std::uint16_t delay=0;
};
OriginalAnimationPlainFrame original_animation_decode_plain_frame(
    OriginalAnimatedEvent& slot,
    const std::vector<std::uint8_t>& original_asset_script);

// Original source ten-slot dispatcher 0x40C50B, but with a strictly
// authenticated non-command script catalog. Original negative opcode
// sequences are explicitly blocked. Scripts are caller-supplied asset
// bytes, not invented visual frames. Catalog must contain the original
// asset pointers for EVERY active slot with ticks == 0.
struct OriginalAnimationScriptAsset {
    std::uint32_t source_pointer=0;
    std::vector<std::uint8_t> script{};
};
OriginalAnimationTimerTick original_animation_tick_with_plain_scripts(
    OriginalAnimationSlots& slots,
    const std::vector<OriginalAnimationScriptAsset>& scripts);

} // namespace sensigolf::recovered
