#pragma once
#include <array>
#include <stdexcept>
#include "sensigolf/classic_shot_model.hpp"
#include "sensigolf/recovered_distance.hpp"

namespace pixtee {
// Nominal full-power range, not a terrain/wind-adjusted landing prediction.
// Run the SAME recovered model on a separate flat neutral surface. No live
// game, gameplay PRNG or original assets are mutated. Cache once per process.
inline const std::array<std::uint32_t,13>& nominal_club_ranges() {
    static const auto ranges=[] {
        std::array<std::uint32_t,13> result{};
        for(std::uint16_t club=0;club<13;++club) {
            sensigolf::ClassicShotModel probe;
            probe.reset();
            probe.set_surface_context({static_cast<std::uint16_t>(club==12?1:0),0,0});
            probe.set_original_interaction_context(nullptr,club==12,32767,32767,false,false);
            sensigolf::ShotInput shot{};
            shot.club_index=club;shot.lie_index=club==12?6:0;
            shot.power_tick=105;shot.accuracy_tick=63;shot.aim_raw=0;
            probe.begin_shot(shot);
            for(int tick=0;tick<4096 && probe.shot_active();++tick)probe.step();
            if(probe.shot_active())throw std::logic_error("nominal club range did not settle");
            result[club]=sensigolf::recovered::distance_to_hole(
                probe.state().x_raw,probe.state().y_raw,0,0,club==12);
        }
        return result;
    }();
    return ranges;
}
}
