#include <cstdlib>
#include <iostream>

#include "sensigolf/recovered_hazard_recovery.hpp"

static long p(const char* s){ return std::strtol(s,nullptr,0); }

int main(int argc,char** argv) {
    if(argc!=8) {
        std::cerr<<"usage: recovered_hazard_recovery_probe "
                 <<"<ball_x> <ball_y> <safe_x> <safe_y> "
                 <<"<x_extent> <y_extent> <green_mode>\n";
        return 2;
    }
    sensigolf::recovered::HazardRecoveryInput in{};
    in.ball_x=static_cast<std::int32_t>(p(argv[1]));
    in.ball_y=static_cast<std::int32_t>(p(argv[2]));
    in.safe_anchor_x=static_cast<std::int32_t>(p(argv[3]));
    in.safe_anchor_y=static_cast<std::int32_t>(p(argv[4]));
    in.x_extent=static_cast<std::int32_t>(p(argv[5]));
    in.y_extent=static_cast<std::int32_t>(p(argv[6]));
    in.green_mode=p(argv[7])!=0;
    const auto r=sensigolf::recovered::recover_hazard_position(in);
    std::cout
      <<"{\"ball_x\":"<<r.ball_x
      <<",\"ball_y\":"<<r.ball_y
      <<",\"player_x\":"<<r.player_x
      <<",\"player_y\":"<<r.player_y
      <<",\"player_last_x\":"<<r.player_last_x
      <<",\"player_last_y\":"<<r.player_last_y
      <<",\"height\":"<<r.height
      <<",\"pause\":"<<r.pause
      <<",\"player_flags\":"<<r.player_flags
      <<"}\n";
    return 0;
}
