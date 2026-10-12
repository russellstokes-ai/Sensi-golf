#include <cstdint>
#include <iostream>
#include <vector>
#include "sensigolf/recovered_green_scene_bounds.hpp"
int main(){
    std::uint32_t x,y,green,n;
    while(std::cin>>x>>y>>green>>n) {
        if(n==0 || n>6)return 2;
        std::vector<sensigolf::recovered::OriginalGreenSceneObject> objects;
        for(std::uint32_t i=0;i<n;++i){
            std::uint32_t xx,yy;
            if(!(std::cin>>xx>>yy))return 2;
            objects.push_back({
                static_cast<std::int16_t>(static_cast<std::uint16_t>(xx)),
                static_cast<std::int16_t>(static_cast<std::uint16_t>(yy))
            });
        }
        std::cout<<(sensigolf::recovered::original_green_scene_all_inside(
          static_cast<std::uint16_t>(x),
          static_cast<std::uint16_t>(y),
          objects,green!=0)?1:0)<<'\n';
    }
}
