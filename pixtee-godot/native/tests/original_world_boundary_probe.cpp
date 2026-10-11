#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_world_boundary.hpp"
int main() {
  std::int32_t x,y,xextent,yextent;
  std::uint32_t flags,hole,total,pause,gate;
  while(std::cin >> x >> y >> xextent >> yextent
                 >> flags >> hole >> total >> pause >> gate) {
    sensigolf::recovered::OriginalWorldBoundaryInput in{};
    in.ball_x=x;in.ball_y=y;in.x_extent=xextent;in.y_extent=yextent;
    in.player_flags=static_cast<std::uint16_t>(flags);
    in.strokes_on_hole=static_cast<std::uint16_t>(hole);
    in.strokes_total=static_cast<std::uint16_t>(total);
    in.ball_pause=static_cast<std::uint16_t>(pause);
    in.game_gate=static_cast<std::uint16_t>(gate);
    auto r=sensigolf::recovered::original_world_boundary_check(in);
    std::cout << (r.outside_world?1:0) << ' ' << r.source_map_ax << ' '
              << r.player_flags << ' ' << r.strokes_on_hole << ' '
              << r.strokes_total << ' ' << r.ball_pause << ' '
              << r.game_gate << ' '
              << (r.requires_out_of_bounds_dispatch?1:0) << '\n';
  }
}
