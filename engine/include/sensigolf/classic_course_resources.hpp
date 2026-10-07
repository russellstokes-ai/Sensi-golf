#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "sensigolf/recovered_course_lookup.hpp"

namespace sensigolf {

struct RawSptRecord {
    std::array<std::uint16_t, 5> words{};
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

    // Read the original big-endian 10-bit tile index from MAPM map data.
    std::uint16_t map_tile(
        std::uint16_t map_x,
        std::uint16_t map_y) const;

    // Run the Gate-1 parity-proven MAPI lookup for one map cell/subcell.
    recovered::CourseLookupResult lookup(
        std::uint16_t map_x,
        std::uint16_t map_y,
        std::uint8_t x_subcell,
        std::uint8_t y_subcell) const;

    // SPT semantics remain deliberately unlabelled until separately evidenced.
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
