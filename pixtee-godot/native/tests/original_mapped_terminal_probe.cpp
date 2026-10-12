// Original source 0x40A65E map-to-event integration: native line protocol.
// Each row holds tile, subcell, 4 descriptor words, 8 selector bytes,
// club, player flags, contact flags, V/H/height. No original art or map.
#include <array>
#include <cstdint>
#include <iostream>
#include <vector>
#include "sensigolf/recovered_mapped_terminal.hpp"

int main() {
    std::uint32_t tile,x,y,club,pf,cf;
    std::array<std::uint32_t,4> words{};
    std::array<std::uint32_t,8> selectors{};
    std::int32_t v,h,z;
    while(std::cin >> tile >> x >> y) {
        for(auto& word:words)std::cin >> word;
        for(auto& mask:selectors)std::cin >> mask;
        if(!(std::cin >> club >> pf >> cf >> v >> h >> z))return 2;
        std::vector<std::uint8_t> desc((tile+1u)*8u);
        std::vector<std::uint8_t> sel((tile+1u)*8u);
        for(unsigned i=0;i<4;++i) {
            desc[tile*8u+2*i]=static_cast<std::uint8_t>(words[i]>>8);
            desc[tile*8u+2*i+1]=static_cast<std::uint8_t>(words[i]);
        }
        for(unsigned i=0;i<8;++i)sel[tile*8u+i]=static_cast<std::uint8_t>(selectors[i]);
        sensigolf::recovered::OriginalMappedTerminalInput in{};
        in.descriptor_bank=desc.data();in.descriptor_size=desc.size();
        in.selector_bank=sel.data();in.selector_size=sel.size();
        // The original selected tile offset remains byte-for-byte identical.
        in.tile_index=static_cast<std::uint16_t>(tile);
        in.x_subcell=static_cast<std::uint8_t>(x);
        in.y_subcell=static_cast<std::uint8_t>(y);
        in.club_index=static_cast<std::uint16_t>(club);
        in.player_flags=static_cast<std::uint16_t>(pf);
        in.contact_flags=static_cast<std::uint16_t>(cf);
        in.vertical_force=v;in.horizontal_force=h;in.ball_height=z;
        auto out=sensigolf::recovered::original_mapped_terminal_contact(in);
        std::cout << (out.resolved?1:0) << ' '
                  << out.descriptor_index << ' ' << out.landing_code << ' '
                  << out.player_flags << ' ' << out.contact_flags << ' '
                  << out.vertical_force << ' ' << out.horizontal_force << ' '
                  << out.ball_height << ' ' << (out.continues_flight?1:0) << ' '
                  << out.event_code << '\n';
    }
}
