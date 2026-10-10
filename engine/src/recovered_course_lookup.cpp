#include "sensigolf/recovered_course_lookup.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace sensigolf::recovered {

CourseLookupResult lookup_course_subcell(
    const std::uint8_t* descriptor_bank,
    std::size_t descriptor_size,
    const std::uint8_t* selector_bank,
    std::size_t selector_size,
    std::uint16_t tile_index,
    std::uint8_t x_subcell,
    std::uint8_t y_subcell) {

    if (!descriptor_bank || !selector_bank) {
        throw std::invalid_argument("course lookup banks must be non-null");
    }
    if (x_subcell > 7 || y_subcell > 3) {
        throw std::out_of_range("course lookup subcell outside 8x4 range");
    }

    const std::size_t tile_offset =
        static_cast<std::size_t>(tile_index) * 8u;
    if (tile_offset + 7u >= descriptor_size
        || tile_offset + 7u >= selector_size) {
        throw std::out_of_range("course lookup tile outside supplied MAPI bank");
    }

    const std::uint8_t mask =
        static_cast<std::uint8_t>(0x80u >> x_subcell);
    std::size_t selected_offset = 0;

    if ((selector_bank[tile_offset + y_subcell] & mask) != 0) {
        selected_offset += 2;
    }
    if ((selector_bank[tile_offset + y_subcell + 4u] & mask) != 0) {
        selected_offset += 4;
    }

    const std::size_t word_offset = tile_offset + selected_offset;
    const std::uint16_t raw =
        static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(descriptor_bank[word_offset]) << 8)
            | descriptor_bank[word_offset + 1u]);

    std::uint16_t descriptor =
        static_cast<std::uint16_t>(raw & 0x00FFu);
    if (descriptor >= 0x004Du) {
        descriptor = 4;
    }

    const std::uint16_t meta =
        static_cast<std::uint16_t>(raw >> 8);
    const std::uint16_t slope_direction =
        static_cast<std::uint16_t>((meta & 0x0Fu) << 8);
    const std::uint16_t slope_magnitude =
        static_cast<std::uint16_t>(meta >> 4);

    return CourseLookupResult{
        descriptor,
        slope_direction,
        slope_magnitude,
        raw,
    };
}

} // namespace sensigolf::recovered
