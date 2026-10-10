#include "sensigolf/classic_course_resources.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

namespace sensigolf {

ClassicCourseResources::ClassicCourseResources(
    std::vector<std::uint8_t> mapm,
    std::vector<std::uint8_t> spt,
    std::vector<std::uint8_t> mapi_descriptor,
    std::vector<std::uint8_t> mapi_selector,
    std::vector<std::uint8_t> maps)
    : mapm_(std::move(mapm)),
      spt_(std::move(spt)),
      maps_(std::move(maps)),
      mapi_descriptor_(std::move(mapi_descriptor)),
      mapi_selector_(std::move(mapi_selector)) {
    if (mapm_.size() < kMapHeaderBytes) {
        throw std::invalid_argument("MAPM resource shorter than recovered header");
    }
    map_width_ = read_be16(mapm_, 0x54);
    map_height_ = read_be16(mapm_, 0x56);

    if (!maps_.empty()) {
        if (maps_.size() < kMapHeaderBytes) {
            throw std::invalid_argument(
                "MAPS resource shorter than recovered header");
        }
        green_map_width_ = read_be16(maps_, 0x54);
        green_map_height_ = read_be16(maps_, 0x56);
    }

    validate();

    // Original v1.014 routine 0x409687 scans MAPM cells in row-major order.
    // Words whose top three bits are set are transition markers. The first
    // marker supplies the green coordinate origin used by 0x40AB1B/0x40AB97.
    std::size_t marker_count = 0;
    for (std::uint16_t y = 0; y < map_height_; ++y) {
        for (std::uint16_t x = 0; x < map_width_; ++x) {
            const auto cell =
                static_cast<std::size_t>(y) * map_width_
                + static_cast<std::size_t>(x);
            const auto raw = read_be16(
                mapm_, kMapHeaderBytes + cell * 2u);
            if ((raw & 0xE000u) == 0xE000u) {
                if (marker_count == 0) {
                    green_region_ = ClassicGreenRegion{
                        static_cast<std::uint16_t>(x << 4),
                        static_cast<std::uint16_t>(y << 3)};
                }
                ++marker_count;
            }
        }
    }

    // The original transition setup expects multiple marker points. Do not
    // enable green-mode switching for synthetic/partial maps that only happen
    // to contain one flagged tile.
    if (marker_count < 3 || maps_.empty()) {
        green_region_.reset();
    }
}

std::uint16_t ClassicCourseResources::read_be16(
    const std::vector<std::uint8_t>& data,
    std::size_t offset) {
    if (offset + 1 >= data.size()) {
        throw std::out_of_range("big-endian word outside resource");
    }
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(data[offset]) << 8)
        | static_cast<std::uint16_t>(data[offset + 1]));
}

void ClassicCourseResources::validate() const {
    if (map_width_ == 0 || map_height_ == 0) {
        throw std::invalid_argument("MAPM dimensions must be non-zero");
    }

    const auto cells =
        static_cast<std::size_t>(map_width_)
        * static_cast<std::size_t>(map_height_);
    const auto required_map_bytes = kMapHeaderBytes + cells * 2u;
    if (mapm_.size() < required_map_bytes) {
        throw std::invalid_argument("MAPM resource truncated for recovered dimensions");
    }

    if (spt_.size() != kSptBytes) {
        throw std::invalid_argument("MAPM SPT resource must be exactly 50 bytes");
    }

    if (mapi_descriptor_.empty()
        || mapi_descriptor_.size() != mapi_selector_.size()
        || (mapi_descriptor_.size() % 8u) != 0) {
        throw std::invalid_argument(
            "MAPI banks must be non-empty, equal-sized and eight-byte aligned");
    }

    const auto tile_count = mapi_tile_count();
    for (std::uint16_t y = 0; y < map_height_; ++y) {
        for (std::uint16_t x = 0; x < map_width_; ++x) {
            if (map_tile(x, y) >= tile_count) {
                throw std::invalid_argument(
                    "MAPM tile index outside supplied MAPI bank pair");
            }
        }
    }

    if (!maps_.empty()) {
        if (green_map_width_ == 0 || green_map_height_ == 0) {
            throw std::invalid_argument("MAPS dimensions must be non-zero");
        }
        const auto green_cells =
            static_cast<std::size_t>(green_map_width_)
            * static_cast<std::size_t>(green_map_height_);
        const auto required_green_bytes =
            kMapHeaderBytes + green_cells * 2u;
        if (maps_.size() < required_green_bytes) {
            throw std::invalid_argument(
                "MAPS resource truncated for recovered dimensions");
        }
        for (std::uint16_t y = 0; y < green_map_height_; ++y) {
            for (std::uint16_t x = 0; x < green_map_width_; ++x) {
                if (map_tile_from(
                        maps_,
                        green_map_width_,
                        green_map_height_,
                        x,
                        y,
                        "MAPS") >= tile_count) {
                    throw std::invalid_argument(
                        "MAPS tile index outside supplied MAPI bank pair");
                }
            }
        }
    }
}

std::uint16_t ClassicCourseResources::map_width() const noexcept {
    return map_width_;
}

std::uint16_t ClassicCourseResources::map_height() const noexcept {
    return map_height_;
}

bool ClassicCourseResources::has_green_map() const noexcept {
    return !maps_.empty() && green_region_.has_value();
}

std::uint16_t ClassicCourseResources::green_map_width() const noexcept {
    return green_map_width_;
}

std::uint16_t ClassicCourseResources::green_map_height() const noexcept {
    return green_map_height_;
}

std::optional<ClassicGreenRegion>
ClassicCourseResources::green_region() const noexcept {
    return green_region_;
}

std::size_t ClassicCourseResources::mapi_tile_count() const noexcept {
    return mapi_descriptor_.size() / 8u;
}

std::int32_t ClassicCourseResources::recovery_x_extent() const noexcept {
    return static_cast<std::int32_t>(map_width_) * 16 - 0x100;
}

std::int32_t ClassicCourseResources::recovery_y_extent() const noexcept {
    return static_cast<std::int32_t>(map_height_) * 8 - 0xD0;
}

std::uint16_t ClassicCourseResources::map_tile_from(
    const std::vector<std::uint8_t>& map,
    std::uint16_t width,
    std::uint16_t height,
    std::uint16_t map_x,
    std::uint16_t map_y,
    const char* label) const {
    if (map_x >= width || map_y >= height) {
        throw std::out_of_range(
            std::string(label) + " cell outside recovered dimensions");
    }

    const auto cell =
        static_cast<std::size_t>(map_y) * width
        + static_cast<std::size_t>(map_x);
    const auto offset = kMapHeaderBytes + cell * 2u;
    return static_cast<std::uint16_t>(
        read_be16(map, offset) & 0x03FFu);
}

std::uint16_t ClassicCourseResources::map_tile(
    std::uint16_t map_x,
    std::uint16_t map_y) const {
    return map_tile_from(
        mapm_, map_width_, map_height_, map_x, map_y, "MAPM");
}

recovered::CourseLookupResult ClassicCourseResources::lookup(
    std::uint16_t map_x,
    std::uint16_t map_y,
    std::uint8_t x_subcell,
    std::uint8_t y_subcell) const {
    const auto tile = map_tile(map_x, map_y);
    return recovered::lookup_course_subcell(
        mapi_descriptor_.data(),
        mapi_descriptor_.size(),
        mapi_selector_.data(),
        mapi_selector_.size(),
        tile,
        x_subcell,
        y_subcell);
}

ResolvedCourseSurface ClassicCourseResources::resolve_surface_from(
    const std::vector<std::uint8_t>& map,
    std::uint16_t width,
    std::uint16_t height,
    std::uint16_t map_x,
    std::uint16_t map_y,
    std::uint8_t x_subcell,
    std::uint8_t y_subcell,
    const char* label) const {
    const auto tile = map_tile_from(
        map, width, height, map_x, map_y, label);
    const auto result = recovered::lookup_course_subcell(
        mapi_descriptor_.data(),
        mapi_descriptor_.size(),
        mapi_selector_.data(),
        mapi_selector_.size(),
        tile,
        x_subcell,
        y_subcell);
    const auto& descriptor =
        classic_terrain_descriptor(result.descriptor_index);

    return ResolvedCourseSurface{
        result.descriptor_index,
        descriptor.landing_code,
        descriptor.variant,
        descriptor.profile_slot,
        descriptor.name,
        result.slope_direction,
        result.slope_magnitude,
        descriptor.product_supported,
    };
}

ResolvedCourseSurface ClassicCourseResources::resolve_surface(
    std::uint16_t map_x,
    std::uint16_t map_y,
    std::uint8_t x_subcell,
    std::uint8_t y_subcell) const {
    return resolve_surface_from(
        mapm_,
        map_width_,
        map_height_,
        map_x,
        map_y,
        x_subcell,
        y_subcell,
        "MAPM");
}

ResolvedCourseSurface ClassicCourseResources::resolve_integer_position(
    std::int16_t integer_x,
    std::int16_t integer_y,
    bool green_mode) const {
    if (integer_x < 0 || integer_y < 0) {
        throw std::out_of_range(
            green_mode
                ? "negative green coordinate outside MAPS"
                : "negative course coordinate outside MAPM");
    }

    const auto ux = static_cast<std::uint16_t>(integer_x);
    const auto uy = static_cast<std::uint16_t>(integer_y);

    // Exact original v1.014 0x409535 dispatcher. Green mode selects MAPS and
    // limits the detailed lookup window to 0x100 x 0xD0 coordinate units.
    if (green_mode) {
        if (!has_green_map()) {
            throw std::logic_error(
                "green-mode lookup requires recovered MAPS resource");
        }
        if (ux >= 0x100u || uy >= 0xD0u) {
            throw std::out_of_range(
                "green coordinate outside original MAPS lookup window");
        }
        return resolve_surface_from(
            maps_,
            green_map_width_,
            green_map_height_,
            static_cast<std::uint16_t>(ux >> 4),
            static_cast<std::uint16_t>(uy >> 3),
            static_cast<std::uint8_t>((ux >> 1) & 7u),
            static_cast<std::uint8_t>((uy >> 1) & 3u),
            "MAPS");
    }

    return resolve_surface(
        static_cast<std::uint16_t>(ux >> 4),
        static_cast<std::uint16_t>(uy >> 3),
        static_cast<std::uint8_t>((ux >> 1) & 7u),
        static_cast<std::uint8_t>((uy >> 1) & 3u));
}

ResolvedCourseSurface ClassicCourseResources::resolve_raw_position(
    std::int32_t x_raw,
    std::int32_t y_raw,
    bool green_mode) const {
    const auto ix = static_cast<std::int16_t>(
        static_cast<std::uint32_t>(x_raw) >> 16);
    const auto iy = static_cast<std::int16_t>(
        static_cast<std::uint32_t>(y_raw) >> 16);
    return resolve_integer_position(ix, iy, green_mode);
}

ClassicCoursePoint ClassicCourseResources::player_start(
    std::size_t player_slot) const {
    if (player_slot >= 4) {
        throw std::out_of_range(
            "player SPT start slot outside recovered 0..3 range");
    }
    const auto row = spt_record(player_slot);
    return ClassicCoursePoint{row.words[2], row.words[3]};
}

ClassicCoursePoint ClassicCourseResources::hole_position() const {
    const auto row = spt_record(4);
    return ClassicCoursePoint{row.words[2], row.words[3]};
}

RawSptRecord ClassicCourseResources::spt_record(std::size_t index) const {
    if (index >= kSptRecordCount) {
        throw std::out_of_range("SPT record outside recovered five-record file");
    }

    RawSptRecord row{};
    const auto base = index * 10u;
    for (std::size_t word = 0; word < row.words.size(); ++word) {
        row.words[word] = read_be16(spt_, base + word * 2u);
    }
    return row;
}

} // namespace sensigolf
