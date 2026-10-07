#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

#include "sensigolf/classic_terrain.hpp"
#include "sensigolf/recovered_course_lookup.hpp"

namespace sensigolf {

struct RawSptRecord {
    std::array<std::uint16_t, 5> words{};
};

struct ResolvedCourseSurface {
    std::uint16_t descriptor_index = 0;
    std::uint16_t landing_code = 0;
    std::int16_t variant = 0;
    std::uint16_t profile_slot = 0;
    std::string_view name{};
    std::uint16_t slope_direction = 0;
    std::uint16_t slope_magnitude = 0;
    bool product_supported = false;
};

class ClassicCourseResources {
public:
    static constexpr std::size_t kMapHeaderBytes = 0x60;
    static constexpr std::size_t kSptBytes = 50;
    static constexpr std::size_t kSptRecordCount = 5;

    ClassicCourseResources(
        std::vector<std::uint8_t> mapm,
        std::vector<std::uint8_t> spt,
        std::vector<std::uint8_t> mapi_descriptor,
        std::vector<std::uint8_t> mapi_selector);

    std::uint16_t map_width() const noexcept;
    std::uint16_t map_height() const noexcept;
    std::size_t mapi_tile_count() const noexcept;

    std::uint16_t map_tile(
        std::uint16_t map_x,
        std::uint16_t map_y) const;

    recovered::CourseLookupResult lookup(
        std::uint16_t map_x,
        std::uint16_t map_y,
        std::uint8_t x_subcell,
        std::uint8_t y_subcell) const;

    ResolvedCourseSurface resolve_surface(
        std::uint16_t map_x,
        std::uint16_t map_y,
        std::uint8_t x_subcell,
        std::uint8_t y_subcell) const;

    // Exact coordinate decomposition recovered at Windows v1.014 0x409535.
    ResolvedCourseSurface resolve_integer_position(
        std::int16_t integer_x,
        std::int16_t integer_y) const;

    // Ball coordinates use the recovered 16.16-style raw representation.
    // The original lookup receives the signed high word of each coordinate.
    ResolvedCourseSurface resolve_raw_position(
        std::int32_t x_raw,
        std::int32_t y_raw) const;

    // SPT semantics stay deliberately raw until separately evidenced.
    RawSptRecord spt_record(std::size_t index) const;

private:
    static std::uint16_t read_be16(
        const std::vector<std::uint8_t>& data,
        std::size_t offset);

    void validate() const;

    std::vector<std::uint8_t> mapm_;
    std::vector<std::uint8_t> spt_;
    std::vector<std::uint8_t> mapi_descriptor_;
    std::vector<std::uint8_t> mapi_selector_;
    std::uint16_t map_width_ = 0;
    std::uint16_t map_height_ = 0;
};

} // namespace sensigolf
