#include "sensigolf/recovered_green_scene_bounds.hpp"
namespace sensigolf::recovered {
namespace {
std::int16_t signed_word(std::uint16_t v) noexcept {
    const auto x=v<=0x7fffu?static_cast<std::int32_t>(v):
        static_cast<std::int32_t>(v)-65536;
    return static_cast<std::int16_t>(x);
}
}
bool original_green_scene_contains(
    std::uint16_t origin_x,std::uint16_t origin_y,
    OriginalGreenSceneObject object,bool green_mode) noexcept {
    // Source 0x40C37B..0x40C391: AX=x+8, BX=y+8;
    // upper CX=x+112, DX=y+84. Source compares original WORD
    // as signed, so intentionally allow wrapped map-origin arithmetic.
    auto left=signed_word(static_cast<std::uint16_t>(origin_x+8u));
    auto right=signed_word(static_cast<std::uint16_t>(origin_x+112u));
    auto top=signed_word(static_cast<std::uint16_t>(origin_y+8u));
    auto bottom=signed_word(static_cast<std::uint16_t>(origin_y+84u));
    if(green_mode) {
        // Source 0x40C3A3..0x40C3C9 (SUB, SHL, ADD):
        // after scaling, visible green-object coordinates are
        // inclusive x[16,226], y[16,170].
        left=16;right=226;top=16;bottom=170;
    }
    return object.x>=left && object.x<=right
        && object.y>=top && object.y<=bottom;
}
bool original_green_scene_all_inside(
    std::uint16_t origin_x,std::uint16_t origin_y,
    const std::vector<OriginalGreenSceneObject>& objects,
    bool green_mode) noexcept {
    // An empty array does not represent an executable original loop:
    // the original executes at least one initial iteration of its BP
    // loop; reject empties as not an authenticated game state.
    if(objects.empty())return false;
    for(const auto object:objects) {
        if(!original_green_scene_contains(
              origin_x,origin_y,object,green_mode)) return false;
    }
    return true;
}
} // namespace sensigolf::recovered
