#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "sensigolf/classic_terrain.hpp"
#include "sensigolf/recovered_course_lookup.hpp"

namespace sensigolf {

struct RawSptRecord {
    std::array<std::uint16_t, 5> words{};
};

struct ClassicCoursePoint {
    std::uint16_t x = 0;
    std::uint16_t y = 0;

    std::int32_t x_raw() const noexcept {
        return static_cast<std::int32_t>(
            static_cast<std::uint32_t>(x) << 16);
    }

    std::int32_t y_raw() const noexcept {
        return static_cast<std::int32_t>(
            static_cast<std::uint32_t>(y) << 16);
    }
};

struct ClassicGreenRegion {
    std::uint16_t origin_x = 0;
    std::uint16_t origin_y = 0;

    bool contains_course(std::int16_t x, std::int16_t y) const noexcept {
        return x >= static_cast<std::int32_t>(origin_x) + 8
            && x <= static_cast<std::int32_t>(origin_x) + 112
            && y >= static_cast<std::int32_t>(origin_y) + 8
            && y <= static_cast<std::int32_t>(origin_y) + 84;
    }

    bool contains_green(std::int16_t x, std::int16_t y) const noexcept {
        return x >= 16 && x <= 226 && y >= 16 && y <= 170;
    }
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
        std::vector<std::uint8_t> mapi_selector,
        std::vector<std::uint8_t> maps = {});

    std::uint16_t map_width() const noexcept;
    std::uint16_t map_height() const noexcept;
    bool has_green_map() const noexcept;
    std::uint16_t green_map_width() const noexcept;
    std::uint16_t green_map_height() const noexcept;
    std::optional<ClassicGreenRegion> green_region() const noexcept;
    std::size_t mapi_tile_count() const noexcept;

    // Exact globals written by the original MAPM loader:
    // x_extent = width*16 - 0x100
    // y_extent = height*8 - 0xD0
    std::int32_t recovery_x_extent() const noexcept;
    std::int32_t recovery_y_extent() const noexcept;

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
        std::int16_t integer_y,
        bool green_mode = false) const;

    // Ball coordinates use the recovered 16.16-style raw representation.
    // The original lookup receives the signed high word of each coordinate.
    ResolvedCourseSurface resolve_raw_position(
        std::int32_t x_raw,
        std::int32_t y_raw,
        bool green_mode = false) const;

    // Recovered original hole setup semantics:
    // records 0..3 words 2/3 are player tee/start coordinates;
    // record 4 words 2/3 are the cup/hole coordinates.
    ClassicCoursePoint player_start(std::size_t player_slot) const;
    ClassicCoursePoint hole_position() const;

    // Other SPT words remain deliberately raw until separately evidenced.
    RawSptRecord spt_record(std::size_t index) const;

private:
    static std::uint16_t read_be16(
        const std::vector<std::uint8_t>& data,
        std::size_t offset);

    void validate() const;

    std::vector<std::uint8_t> mapm_;
    std::vector<std::uint8_t> spt_;
    std::vector<std::uint8_t> maps_;
    std::vector<std::uint8_t> mapi_descriptor_;
    std::vector<std::uint8_t> mapi_selector_;
    std::uint16_t map_width_ = 0;
    std::uint16_t map_height_ = 0;
    std::uint16_t green_map_width_ = 0;
    std::uint16_t green_map_height_ = 0;
    std::optional<ClassicGreenRegion> green_region_{};

    std::uint16_t map_tile_from(
        const std::vector<std::uint8_t>& map,
        std::uint16_t width,
        std::uint16_t height,
        std::uint16_t map_x,
        std::uint16_t map_y,
        const char* label) const;
    ResolvedCourseSurface resolve_surface_from(
        const std::vector<std::uint8_t>& map,
        std::uint16_t width,
        std::uint16_t height,
        std::uint16_t map_x,
        std::uint16_t map_y,
        std::uint8_t x_subcell,
        std::uint8_t y_subcell,
        const char* label) const;
};

} // namespace sensigolf
