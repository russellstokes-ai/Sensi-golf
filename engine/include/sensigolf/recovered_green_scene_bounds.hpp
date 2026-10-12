#pragma once
#include <cstdint>
#include <vector>

namespace sensigolf::recovered {
// Original Windows v1.014 0x40C35B..0x40C3F0 / 0x40C40D:
// the actual stage GREEN-map admission loop examines EVERY active original
// course object, not just the ball or hole position. Source uses signed
// 16-bit JL/JG inclusive bounds, 16-bit wrapping additions and two
// coordinate systems. This does NOT identify tree sprite hitboxes.
struct OriginalGreenSceneObject {
    std::int16_t x = 0; // source [0x4287D6+i*0x2C+2]
    std::int16_t y = 0; // source [0x4287D6+i*0x2C+6]
};
bool original_green_scene_contains(
    std::uint16_t origin_x,std::uint16_t origin_y,
    OriginalGreenSceneObject object,bool green_mode) noexcept;
bool original_green_scene_all_inside(
    std::uint16_t origin_x,std::uint16_t origin_y,
    const std::vector<OriginalGreenSceneObject>& objects,
    bool green_mode) noexcept;
} // namespace sensigolf::recovered
