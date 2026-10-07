#include "sensigolf/recovered_hazard_recovery.hpp"

#include <cstdint>

namespace sensigolf::recovered {
namespace {

std::int16_t high_word(std::int32_t value) noexcept {
    return static_cast<std::int16_t>(
        static_cast<std::uint32_t>(value) >> 16);
}

std::int32_t logical_shift_right_one(std::int32_t value) noexcept {
    return static_cast<std::int32_t>(
        static_cast<std::uint32_t>(value) >> 1);
}

} // namespace

HazardRecoveryState recover_hazard_position(
    const HazardRecoveryInput& input) noexcept {
    HazardRecoveryState out{};
    out.ball_x = input.ball_x;
    out.ball_y = input.ball_y;

    const auto ix = high_word(input.ball_x);
    const auto iy = high_word(input.ball_y);
    const auto x_limit =
        static_cast<std::int64_t>(input.x_extent) + 0x100;
    const auto y_limit =
        static_cast<std::int64_t>(input.y_extent) + 0xD0;

    const bool outside =
        ix < 0 || iy < 0
        || static_cast<std::int64_t>(ix) > x_limit
        || static_cast<std::int64_t>(iy) > y_limit;

    std::int32_t px=0;
    std::int32_t py=0;

    if (!outside) {
        px=input.ball_x;
        py=input.ball_y;
        if (input.green_mode) {
            px=logical_shift_right_one(px);
            py=logical_shift_right_one(py);
        }
        px=static_cast<std::int32_t>(
            static_cast<std::uint32_t>(px)-0x000F0000u);
        py=static_cast<std::int32_t>(
            static_cast<std::uint32_t>(py)-0x000F0000u);
    } else {
        out.used_safe_anchor=true;
        out.ball_x=input.safe_anchor_x;
        out.ball_y=input.safe_anchor_y;

        px=input.safe_anchor_x;
        py=input.safe_anchor_y;
        if (input.green_mode) {
            px=static_cast<std::int32_t>(
                static_cast<std::uint32_t>(px)-0x000F0000u);
            py=static_cast<std::int32_t>(
                static_cast<std::uint32_t>(py)-0x000F0000u);
            px=logical_shift_right_one(px);
            py=logical_shift_right_one(py);
        }

        out.ball_x=static_cast<std::int32_t>(
            static_cast<std::uint32_t>(out.ball_x)+0x000F0000u);
        out.ball_y=static_cast<std::int32_t>(
            static_cast<std::uint32_t>(out.ball_y)+0x000F0000u);
    }

    out.player_x=px;
    out.player_y=py;
    out.player_last_x=px;
    out.player_last_y=py;
    out.height=0;
    out.pause=0x003C;
    out.player_flags=0;
    return out;
}

} // namespace sensigolf::recovered
