#include <cstdint>
#include <iostream>
int main(){
    std::uint32_t x,y;
    while(std::cin>>x>>y) {
        // Recovered original fifth SPT record words +4/+6, both
        // stored big-endian on disk. This tiny original format probe
        // avoids bundling any copyrighted original game assets.
        const auto b0=static_cast<std::uint8_t>(x>>8);
        const auto b1=static_cast<std::uint8_t>(x);
        const auto c0=static_cast<std::uint8_t>(y>>8);
        const auto c1=static_cast<std::uint8_t>(y);
        const auto xx=static_cast<std::uint16_t>((b0<<8)|b1);
        const auto yy=static_cast<std::uint16_t>((c0<<8)|c1);
        std::cout<<xx<<' '<<yy<<' '<<3<<'\n';
    }
}
