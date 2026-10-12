#include <cstdint>
#include <iostream>
#include <vector>
#include "sensigolf/recovered_flag_setup.hpp"
static void be16(std::vector<std::uint8_t>& b,std::size_t at,
                 std::uint16_t x) {
    b.at(at)=static_cast<std::uint8_t>(x>>8);
    b.at(at+1)=static_cast<std::uint8_t>(x);
}
int main(){
    std::uint32_t x,y;
    while(std::cin>>x>>y){
        std::vector<std::uint8_t> map(0x60+4*128*2,0),spt(50,0);
        std::vector<std::uint8_t> desc(8,0),selector(8,0);
        be16(map,0x54,4);be16(map,0x56,128);
        be16(spt,44,static_cast<std::uint16_t>(x));
        be16(spt,46,static_cast<std::uint16_t>(y));
        sensigolf::ClassicCourseResources course(
            std::move(map),std::move(spt),
            std::move(desc),std::move(selector));
        const auto out=sensigolf::recovered::original_flag_setup_from_spt(course);
        std::cout<<out.x<<' '<<out.y<<' '<<out.source_marker<<'\n';
    }
}
