// Original source 0x40A65E map-to-event integration: native line protocol.
// Each row holds tile, subcell, 4 descriptor words, 8 selector bytes,
// club, player flags, contact flags, V/H/height. No original art or map.
#include <array>
#include <cstdint>
#include <iostream>
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
        std::array<std::uint8_t,8> desc{},sel{};
        for(unsigned i=0;i<4;++i) {
            desc[2*i]=static_cast<std::uint8_t>(words[i]>>8);
            desc[2*i+1]=static_cast<std::uint8_t>(words[i]);
        }
        for(unsigned i=0;i<8;++i)sel[i]=static_cast<std::uint8_t>(selectors[i]);
        sensigolf::recovered::OriginalMappedTerminalInput in{};
        in.descriptor_bank=desc.data();in.descriptor_size=desc.size();
        in.selector_bank=sel.data();in.selector_size=sel.size();
        // Tile 0 is the selected record; original map tiles vary and will
        // be exercised by the emulator using the same 8-byte record.
        in.tile_index=0;
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
