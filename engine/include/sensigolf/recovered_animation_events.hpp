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

// Original source ten-slot dispatcher 0x40C50B using caller-supplied
// asset scripts. Supports ordinary frames and verified negative commands
// 0x8004 (close) and 0x8008 (wait). Other commands, especially external
// callback 0x8000, are blocked. Assets must be supplied for EVERY
// active zero-tick slot. This does not generate visual/audio output.
struct OriginalAnimationScriptAsset {
    std::uint32_t source_pointer=0;
    std::vector<std::uint8_t> script{};
};
OriginalAnimationTimerTick original_animation_tick_with_plain_scripts(
    OriginalAnimationSlots& slots,
    const std::vector<OriginalAnimationScriptAsset>& scripts);

// The original signed animation-script command jump table
// [0x41F89C] has been identified directly in authenticated source:
// 0x8000 -> 0x40C56A invokes an external callback (UNSUPPORTED);
// 0x8004 -> 0x40C573 closes/releases this animation slot;
// 0x8008 -> 0x40C58D reads the following signed-independent delay
// WORD and advances original script offset by two further bytes.
// Original nonnegative frame decoder remains separately callable.
enum class OriginalAnimationSupportedCommand : std::uint8_t {
    PlainFrame=0, CloseSlot=1, WaitTicks=2
};
struct OriginalAnimationSupportedStep {
    OriginalAnimationSupportedCommand kind=OriginalAnimationSupportedCommand::PlainFrame;
    std::uint16_t frame=0;
    std::uint16_t delay=0;
};
OriginalAnimationSupportedStep original_animation_decode_supported_step(
    OriginalAnimatedEvent& slot,
    const std::vector<std::uint8_t>& original_asset_script);
} // namespace sensigolf::recovered
