#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "pixtee_round_runtime.hpp"
#include "pixtee_course_adapter.hpp"

namespace {
pixtee::PixteeRoundRuntime preview() {
    std::array<std::uint8_t,18> order{};
    order.fill(1u);
    std::array<std::uint8_t,100> pars{};
    pars[1]=4u;
    return pixtee::PixteeRoundRuntime(order,pars,0x1234,0xabcd);
}
} // namespace

int main() {
    // The source-authored fine-map transfer is tested at ALL verified 16x16
    // slope direction and strength combinations. Godot must never guess
    // these values from a screenshot or one arrow sprite orientation.
    auto empty=preview();
    assert(empty.verified_native_green_arrows().empty());
    for(std::uint8_t direction=0;direction<16;++direction) {
        for(std::uint8_t magnitude=0;magnitude<16;++magnitude) {
            auto session=preview();
            const auto source=pixtee::make_lakewood_hole_1_with_slope(
                direction,magnitude);
            session.register_course(1u,source);
            session.load_next_hole();
            const auto origin=session.active_green_region();
            assert(origin && origin->origin_x==96u && origin->origin_y==40u);
            const auto arrows=session.verified_native_green_arrows(8u);
            if(magnitude==0) {
                assert(arrows.empty());
                continue;
            }
            assert(!arrows.empty() && arrows.size()<2048u);
            for(const auto& arrow:arrows) {
                assert(arrow.landing_code==1u);
                assert(arrow.slope_direction_raw==
                       static_cast<std::uint16_t>(direction)*256u);
                assert(arrow.slope_magnitude==magnitude);
                assert(arrow.world_x>=96 && arrow.world_x<224);
                assert(arrow.world_y>=40 && arrow.world_y<144);
                // Original v1.014 scaled MAPS x/y at 2x within the marker
                // rectangle. Every arrow must be exactly reproducible by
                // the recovered native C++ MAPI fine-cell lookup.
                const auto resolved=source.resolve_integer_position(
                    static_cast<std::int16_t>(
                        (arrow.world_x-origin->origin_x)*2),
                    static_cast<std::int16_t>(
                        (arrow.world_y-origin->origin_y)*2),true);
                assert(resolved.landing_code==arrow.landing_code);
                assert(resolved.slope_direction==arrow.slope_direction_raw);
                assert(resolved.slope_magnitude==arrow.slope_magnitude);
            }
            bool bad=false;
            try {
                (void)session.verified_native_green_arrows(0u);
            } catch(const std::invalid_argument&) {bad=true;}
            assert(bad);
        }
    }
    // This tests only the historic temporary native authoring fixture.
    // Source Work Lakewood Green geometry (544x912) still requires exact,
    // no-stretch integration and real tee-to-hole Android acceptance.
    return 0;
}
