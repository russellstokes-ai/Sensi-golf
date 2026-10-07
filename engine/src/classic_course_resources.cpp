#include "sensigolf/classic_course_resources.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>

namespace sensigolf {

ClassicCourseResources::ClassicCourseResources(
    std::vector<std::uint8_t> mapm,
    std::vector<std::uint8_t> spt,
    std::vector<std::uint8_t> mapi_descriptor,
    std::vector<std::uint8_t> mapi_selector)
    : mapm_(std::move(mapm)),
      spt_(std::move(spt)),
      mapi_descriptor_(std::move(mapi_descriptor)),
      mapi_selector_(std::move(mapi_selector)) {
    if (mapm_.size() < kMapHeaderBytes) {
        throw std::invalid_argument("MAPM resource shorter than recovered header");
    }
    map_width_ = read_be16(mapm_, 0x54);
    map_height_ = read_be16(mapm_, 0x56);
    validate();
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

    // Fail at load time rather than later during a shot if MAPM references a
    // tile that the supplied MAPI bank pair cannot resolve.
    const auto tile_count = mapi_tile_count();
    for (std::uint16_t y = 0; y < map_height_; ++y) {
        for (std::uint16_t x = 0; x < map_width_; ++x) {
            if (map_tile(x, y) >= tile_count) {
                throw std::invalid_argument(
                    "MAPM tile index outside supplied MAPI bank pair");
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

std::size_t ClassicCourseResources::mapi_tile_count() const noexcept {
    return mapi_descriptor_.size() / 8u;
}

std::uint16_t ClassicCourseResources::map_tile(
    std::uint16_t map_x,
    std::uint16_t map_y) const {
    if (map_x >= map_width_ || map_y >= map_height_) {
        throw std::out_of_range("MAPM cell outside recovered dimensions");
    }

    const auto cell =
        static_cast<std::size_t>(map_y) * map_width_
        + static_cast<std::size_t>(map_x);
    const auto offset = kMapHeaderBytes + cell * 2u;
    return static_cast<std::uint16_t>(read_be16(mapm_, offset) & 0x03FFu);
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
