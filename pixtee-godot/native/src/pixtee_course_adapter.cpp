#include "pixtee_course_adapter.hpp"
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

namespace pixtee {
namespace {
void write_be16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t v) {
    bytes.at(offset) = static_cast<std::uint8_t>(v >> 8);
    bytes.at(offset + 1) = static_cast<std::uint8_t>(v & 255u);
}
bool sand(float x, float y, float cx, float cy, float rx, float ry) {
    const float dx = (x - cx) / rx, dy = (y - cy) / ry;
    return dx * dx + dy * dy <= 1.f;
}
bool creek(float x, float y) {
    // Pixtee-authored Lakewood polygon from PixteeCourses.kt, not an original-game layout.
    static constexpr float points[][2] = {
        {233,45},{226,68},{230,95},{225,126},{238,161},{231,195},
        {238,232},{224,268},{233,308},{239,345},{231,382},
        {242,422},{299,430},{299,45}
    };
    bool inside = false;
    for (int i = 0, j = 13; i < 14; j = i++) {
        const auto a = points[i], b = points[j];
        if ((a[1] > y) != (b[1] > y) &&
            x < (b[0] - a[0]) * (y - a[1]) / (b[1] - a[1]) + a[0])
            inside = !inside;
    }
    return inside;
}
std::uint16_t descriptor_at(float x, float y) {
    const float gx = (x - 142.f) / 30.f, gy = (y - 82.f) / (30.f * .82f);
    if (gx * gx + gy * gy < 1.f) return 30; // supported GREEN H4
    if (creek(x, y)) return 6;               // water / terminal code 35
    if (sand(x,y,91,141,19,20) || sand(x,y,189,133,20,21) ||
        sand(x,y,200,289,23,21)) return 5; // real sand lie
    if (sand(x,y,150,458,14,14)) return 1; // tee uses fairway-like contact
    const float travel = (458.f - y) / (458.f - 82.f);
    const float t = travel < 0.f ? 0.f : (travel > 1.f ? 1.f : travel);
    const float centre = 150.f + (142.f - 150.f) * t -
        18.f * std::sin(t * 3.14159265f);
    if (y >= 82.f && y <= 463.f && std::fabs(x - centre) < 43.f)
        return 1;
    return 3; // rough, no invented hard tree colliders
}
}
sensigolf::ClassicCourseResources make_lakewood_hole_1() {
    // 19x64 cells span the 300x510 independent Pixtee world at 16x8
    // original-style lookup tiles. This is a first-pass *coarse* terrain
    // conversion. It is NOT approved collision-parity for the artwork.
    constexpr std::uint16_t w = 19, h = 64;
    std::vector<std::uint8_t> map(0x60u + w * h * 2u, 0);
    write_be16(map, 0x54, w);
    write_be16(map, 0x56, h);
    for (unsigned yy = 0; yy < h; ++yy) {
        for (unsigned xx = 0; xx < w; ++xx) {
            const float cx = xx * 16.f + 8.f;
            const float cy = yy * 8.f + 4.f;
            write_be16(map, 0x60u + 2u * (yy * w + xx), descriptor_at(cx,cy));
        }
    }
    std::vector<std::uint8_t> spt(50, 0);
    for (unsigned player = 0; player < 4; ++player) {
        write_be16(spt, player * 10u + 4u, 150);
        write_be16(spt, player * 10u + 6u, 458);
    }
    write_be16(spt, 44, 142);
    write_be16(spt, 46, 82);
    // One uniform selector-record per terrain descriptor. The authoritative
    // recovered MAPI lookup reads the appropriate descriptor and lie profile.
    std::vector<std::uint8_t> desc(77u * 8u, 0), sel(desc.size(), 0);
    for (std::uint16_t index = 0; index < 77; ++index)
        for (unsigned corner = 0; corner < 4; ++corner)
            write_be16(desc, static_cast<std::size_t>(index) * 8u +
                       corner * 2u, index);
    return sensigolf::ClassicCourseResources(
        std::move(map), std::move(spt), std::move(desc), std::move(sel));
}
} // namespace pixtee
