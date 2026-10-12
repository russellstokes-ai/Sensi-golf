#include "original_cpu_candidate.hpp"
#include <iostream>
int main(){
    int count;
    while(std::cin>>count){
        std::vector<pixtee::OriginalComputerCandidate> rows;
        for(int i=0;i<count;++i){
            int a,b,metric;
            std::cin>>a>>b>>metric;
            rows.push_back({
                static_cast<std::int16_t>(a),
                static_cast<std::int16_t>(b),
                static_cast<std::int16_t>(metric)});
        }
        try{std::cout<<pixtee::original_cpu_candidate_index(rows)<<"\n";}
        catch(const std::exception&){std::cout<<"ERR\n";}
    }
}
