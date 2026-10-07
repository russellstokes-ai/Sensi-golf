#include "sensigolf/classic_hole_plan.hpp"

#include <stdexcept>
#include <utility>

namespace sensigolf {
namespace {

std::string resource_name(
    const char* prefix,
    std::uint8_t resource_id,
    const char* suffix) {
    std::string out(prefix);
    out.push_back(static_cast<char>('0' + resource_id / 10u));
    out.push_back(static_cast<char>('0' + resource_id % 10u));
    out += suffix;
    return out;
}

} // namespace

ClassicHolePlan::ClassicHolePlan(
    std::array<std::uint8_t, kClassicRoundHoleCount> order,
    std::array<std::uint8_t, kClassicParTableSize> par_by_resource_id)
    : order_(std::move(order)),
      par_by_resource_id_(std::move(par_by_resource_id)) {
    for (const auto resource_id : order_) {
        if (resource_id == 0u
            || resource_id >= kClassicParTableSize) {
            throw std::invalid_argument(
                "classic hole resource id must be in original 01..99 range");
        }
        if (par_by_resource_id_[resource_id] == 0u) {
            throw std::invalid_argument(
                "classic hole plan resource is missing original par metadata");
        }
    }
}

ClassicHolePlanEntry ClassicHolePlan::hole(
    std::size_t round_index) const {
    if (round_index >= order_.size()) {
        throw std::out_of_range(
            "classic round hole index outside recovered 0..17 range");
    }

    const auto resource_id = order_[round_index];
    return ClassicHolePlanEntry{
        static_cast<std::uint16_t>(round_index),
        resource_id,
        par_by_resource_id_[resource_id],
        resource_names(resource_id),
    };
}

const std::array<std::uint8_t, kClassicRoundHoleCount>&
ClassicHolePlan::order() const noexcept {
    return order_;
}

ClassicHoleResourceNames ClassicHolePlan::resource_names(
    std::uint8_t resource_id) {
    if (resource_id == 0u
        || resource_id >= kClassicParTableSize) {
        throw std::out_of_range(
            "classic hole resource id must be in original 01..99 range");
    }

    // Exact v1.014 templates at 0x41EF00/0x41EF0B/0x41EF16.
    return ClassicHoleResourceNames{
        resource_name("mapm", resource_id, ".map"),
        resource_name("maps", resource_id, ".map"),
        resource_name("mapm", resource_id, ".spt"),
    };
}

} // namespace sensigolf
