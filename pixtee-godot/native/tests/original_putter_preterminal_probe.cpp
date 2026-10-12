// Native original x86 0x40A575..0x40A5F2 pre-putter terrain admission.
#include <array>
#include <cstdint>
#include <iostream>
#include <vector>
#include "sensigolf/recovered_mapped_terminal.hpp"
int main() {
    std::uint32_t tile,x,y,club,pf,cf,bounce;
    std::array<std::uint32_t,4> words{};
    std::array<std::uint32_t,8> masks{};
    std::int32_t z;
    while(std::cin >> tile >> x >> y) {
        for(auto& v:words)std::cin >> v;
        for(auto& v:masks)std::cin >> v;
        if(!(std::cin >> club >> pf >> cf >> z >> bounce))return 2;
        std::vector<std::uint8_t> desc((tile+1u)*8u),sel((tile+1u)*8u);
        for(unsigned i=0;i<4;i++) {
            desc[tile*8u+i*2]=static_cast<std::uint8_t>(words[i]>>8);
            desc[tile*8u+i*2+1]=static_cast<std::uint8_t>(words[i]);
        }
        for(unsigned i=0;i<8;i++)
            sel[tile*8u+i]=static_cast<std::uint8_t>(masks[i]);
        sensigolf::recovered::OriginalMappedTerminalInput in{};
        in.descriptor_bank=desc.data();in.descriptor_size=desc.size();
        in.selector_bank=sel.data();in.selector_size=sel.size();
        in.tile_index=static_cast<std::uint16_t>(tile);
        in.x_subcell=static_cast<std::uint8_t>(x);
        in.y_subcell=static_cast<std::uint8_t>(y);
        in.club_index=static_cast<std::uint16_t>(club);
        in.player_flags=static_cast<std::uint16_t>(pf);
        in.contact_flags=static_cast<std::uint16_t>(cf);
        in.ball_height=z;
        const auto out=sensigolf::recovered::original_putter_terrain_admission(
            in,static_cast<std::uint8_t>(bounce));
        std::cout << static_cast<int>(out.route) << ' '
                  << out.contact_flags << ' '
                  << (out.map_lookup_performed?1:0) << ' '
                  << out.descriptor_index << ' '
                  << out.landing_code << '\n';
    }
}
