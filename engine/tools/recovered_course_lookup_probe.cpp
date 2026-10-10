#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include "sensigolf/recovered_course_lookup.hpp"

using namespace sensigolf::recovered;

namespace {
std::vector<std::uint8_t> read_file(const char* path) {
    std::ifstream in(path,std::ios::binary);
    if(!in) throw std::runtime_error(std::string("cannot open ")+path);
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>());
}
}

int main(int argc,char** argv) {
    if(argc!=3) {
        std::cerr<<"usage: recovered_course_lookup_probe <descriptor_mapi.raw> <selector_mapi.raw>\n";
        return 2;
    }
    try {
        const auto desc=read_file(argv[1]);
        const auto sel=read_file(argv[2]);
        const auto tile_count=
            static_cast<std::uint16_t>(std::min(desc.size(),sel.size())/8u);

        std::cout<<"tile,x,y,descriptor,direction,magnitude\n";
        for(std::uint16_t tile=0;tile<tile_count;++tile) {
            for(std::uint8_t y=0;y<4;++y) {
                for(std::uint8_t x=0;x<8;++x) {
                    const auto r=lookup_course_subcell(
                        desc.data(),desc.size(),sel.data(),sel.size(),tile,x,y);
                    std::cout<<tile<<','<<static_cast<unsigned>(x)<<','
                             <<static_cast<unsigned>(y)<<','
                             <<r.descriptor_index<<','<<r.slope_direction<<','
                             <<r.slope_magnitude<<'\n';
                }
            }
        }
        return 0;
    } catch(const std::exception& e) {
        std::cerr<<e.what()<<"\n";
        return 1;
    }
}
