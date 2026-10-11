#include <cstdint>
#include <iostream>
#include "sensigolf/recovered_world_boundary.hpp"
int main(){
    std::int32_t x,y,sx,sy,xext,yext;
    std::uint32_t mode5a,mode5c,mode62;
    while(std::cin >> x >> y >> sx >> sy >> xext >> yext
                   >> mode5a >> mode5c >> mode62){
        sensigolf::recovered::OriginalWorldOobRestoreInput in{};
        in.coordinates.ball_x=x; in.coordinates.ball_y=y;
        in.coordinates.safe_anchor_x=sx;in.coordinates.safe_anchor_y=sy;
        in.coordinates.x_extent=xext;in.coordinates.y_extent=yext;
        in.player_mode_5a=static_cast<std::uint16_t>(mode5a);
        in.player_mode_5c=static_cast<std::uint16_t>(mode5c);
        in.player_mode_62=static_cast<std::uint16_t>(mode62);
        const auto out=sensigolf::recovered::original_world_oob_restore(in);
        const auto& r=out.relocated;
        std::cout << r.ball_x << ' ' << r.ball_y << ' '
                  << r.player_x << ' ' << r.player_y << ' '
                  << r.player_last_x << ' ' << r.player_last_y << ' '
                  << r.height << ' ' << r.pause << ' '
                  << r.player_flags << ' ' << out.player_mode_5c << ' '
                  << out.player_mode_62 << '\n';
    }
}
