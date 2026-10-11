#include "original_cpu_difficulty.hpp"
#include <iostream>
int main(){
    unsigned mode,flag;
    while(std::cin>>mode>>flag){
        try{
            const auto r=pixtee::original_cpu_search_parameters(
                static_cast<std::uint16_t>(mode), flag!=0);
            std::cout<<r.candidate_stride<<" "<<r.variation_budget<<" "<<r.search_budget<<"\n";
        }catch(const std::exception&){std::cout<<"ERR\n";}
    }
}
