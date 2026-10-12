#pragma once

#include <cstddef>
#include <cstdint>

namespace sensigolf::recovered {

// Source-scoped Windows v1.014 0x40A65E..0x40A7E0:
// 0x409535 MAPI lookup -> 0x41DFF7 terrain descriptor table ->
// code-35 hard stop, code-50 effect, or original code-0 contact latch.
// This enters AFTER upstream shot/flight dispatch; this is NOT a
// detector for trees or non-terrain objects. Code 8 is unimplemented.
struct OriginalMappedTerminalInput {
    const std::uint8_t* descriptor_bank = nullptr;
    std::size_t descriptor_size = 0;
    const std::uint8_t* selector_bank = nullptr;
    std::size_t selector_size = 0;
    std::uint16_t tile_index = 0;
    std::uint8_t x_subcell = 0;
    std::uint8_t y_subcell = 0;
    std::uint16_t club_index = 0;
    std::uint16_t player_flags = 0; // source [ESI+0]
    std::uint16_t contact_flags = 0; // source [ESI+0x60]
    std::int32_t vertical_force = 0;
    std::int32_t horizontal_force = 0;
    std::int32_t ball_height = 0;
};

struct OriginalMappedTerminalResult {
    bool resolved = false; // does NOT mean whole collision resolved
    std::uint16_t descriptor_index = 0;
    std::uint16_t landing_code = 0;
    std::uint16_t player_flags = 0;
    std::uint16_t contact_flags = 0;
    std::int32_t vertical_force = 0;
    std::int32_t horizontal_force = 0;
    std::int32_t ball_height = 0;
    bool continues_flight = false;
    int event_code = -1; // -1 = no event call; 0,1,11 original event arg
};

// Only call when the original 0x40A65E path is known to be reached.
// Skips source code-8 cup handling and zero-vertical-force idle path.
// The mapping is injected, not an assertion of source course-object presence.
OriginalMappedTerminalResult original_mapped_terminal_contact(
    const OriginalMappedTerminalInput& input);


// Source-original v1.014 0x40A575..0x40A5F2 is the entry preceding
// the putter-specific MAPI terrain branch. This is an isolated ADMISSION
// classifier: it intentionally stops *before* any downstream terminal
// or player/scoring effects. Never treat this as a full shot dispatcher.
enum class OriginalPutterAdmissionRoute : std::uint8_t {
    NormalFlight = 0,          // 0x40A603, no terrain lookup
    RollingSurface = 1,        // 0x40A5F7, landing code <8
    CupCode8 = 2,              // 0x40A730
    SpecialGreenCode9Or10 = 3, // 0x40A40B
    Code35LowHeight = 4,      // 0x40A6E3
    HighCodeEvent1 = 5,       // 0x40A6AB
};
struct OriginalPutterAdmissionResult {
    OriginalPutterAdmissionRoute route =
        OriginalPutterAdmissionRoute::NormalFlight;
    std::uint16_t contact_flags = 0;
    bool map_lookup_performed = false;
    std::uint16_t descriptor_index = 0xffffu;
    std::uint16_t landing_code = 0xffffu;
};
// Original source global [0x41F886] is specifically compared to value 1.
// Values other than 1 do NOT bypass the putter map branch.
OriginalPutterAdmissionResult original_putter_terrain_admission(
    const OriginalMappedTerminalInput& input,
    std::uint8_t original_bounce_marker);

} // namespace sensigolf::recovered
