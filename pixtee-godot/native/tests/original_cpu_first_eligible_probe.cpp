#include <cstdint>
#include <iostream>
#include <vector>
#include "original_cpu_score_scratch.hpp"
int main() {
    std::uint32_t span,step,valid_index,seed;
    while(std::cin>>span>>step>>valid_index>>seed) {
        if(span>65535 || step>65535 || valid_index>65535 || seed>65535)
            return 2;
        std::vector<std::uint8_t> scratch(
            pixtee::kOriginalCpuScoreRecordBytes*
            pixtee::kOriginalCpuScoreSafeRecords,0);
        for(std::size_t i=0;i<pixtee::kOriginalCpuScoreSafeRecords;++i) {
            std::uint16_t primary,secondary;
            if(i==valid_index) {
                primary=static_cast<std::uint16_t>(seed&0x7fffu);
                secondary=static_cast<std::uint16_t>((seed*3u)&0x7fffu);
            }else if((i+seed)%2u) {
                primary=0xffffu;secondary=0u;
            }else{
                primary=0u;secondary=0x8000u;
            }
            const auto pos=i*pixtee::kOriginalCpuScoreRecordBytes;
            scratch[pos]=static_cast<std::uint8_t>(primary);
            scratch[pos+1]=static_cast<std::uint8_t>(primary>>8);
            scratch[pos+2]=static_cast<std::uint8_t>(secondary);
            scratch[pos+3]=static_cast<std::uint8_t>(secondary>>8);
        }
        const auto r=pixtee::original_cpu_first_eligible_score(
            static_cast<std::uint16_t>(span),
            static_cast<std::uint16_t>(step),scratch);
        std::cout<<(r.found?1:0)<<' '<<r.original_index<<' '
                 <<r.scanned_cx<<' '<<r.scanned_bx<<' '
                 <<r.scanned_dx<<'\n';
    }
    return 0;
}
