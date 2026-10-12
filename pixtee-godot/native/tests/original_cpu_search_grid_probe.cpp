#include "original_cpu_search_grid.hpp"
#include <iostream>
int main(){
    unsigned mode,band,w0,w1,w2,w3;
    while(std::cin>>mode>>band>>w0>>w1>>w2>>w3) {
        try {
            const auto r=pixtee::original_cpu_scan_grid(
                static_cast<std::uint16_t>(mode),
                static_cast<std::uint8_t>(band),
                {static_cast<std::uint16_t>(w0),static_cast<std::uint16_t>(w1),
                 static_cast<std::uint16_t>(w2),static_cast<std::uint16_t>(w3)});
            std::cout<<r.table_word_0<<" "<<r.table_word_1<<" "<<r.table_word_2<<" "<<r.table_word_3<<" "
                     <<r.spacing<<" "<<r.initial_remainder<<"\n";
        }catch(const std::exception&){std::cout<<"ERR\n";}
    }
}
