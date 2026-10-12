#include <cstdint>
#include <iostream>
#include <vector>
#include "original_cpu_score_scratch.hpp"
int main() {
    std::uint32_t span,step,seed,mode;
    while(std::cin>>span>>step>>seed>>mode) {
        if(span>65535 || step>65535 || seed>65535 || mode>5)return 2;
        std::vector<std::uint8_t> scratch(
            pixtee::kOriginalCpuScoreRecordBytes*
            pixtee::kOriginalCpuScoreSafeRecords,0);
        for(std::size_t i=0;i<pixtee::kOriginalCpuScoreSafeRecords;++i) {
            std::uint16_t a=0,b=0,score=0;
            switch(mode) {
            case 0: score=static_cast<std::uint16_t>(
                0x7fffu-static_cast<std::uint32_t>(i)*31u);break;
            case 1: score=100;break;
            case 2: a=0xffffu;score=static_cast<std::uint16_t>(i);break;
            case 3:
                a=(i+seed)%3u?0u:0xffffu;
                b=(i+seed)%5u?0u:0x8000u;
                score=static_cast<std::uint16_t>(
                    seed*17u+static_cast<std::uint32_t>(i)*7919u);
                break;
            case 4:
                a=(i==seed%100u)?0u:0xffffu;
                score=static_cast<std::uint16_t>(i*137u);
                break;
            case 5:
                score=static_cast<std::uint16_t>(
                    1000u-static_cast<std::uint32_t>(i)*500u);
                break;
            }
            const auto pos=i*pixtee::kOriginalCpuScoreRecordBytes;
            scratch[pos]=static_cast<std::uint8_t>(a);
            scratch[pos+1]=static_cast<std::uint8_t>(a>>8);
            scratch[pos+2]=static_cast<std::uint8_t>(b);
            scratch[pos+3]=static_cast<std::uint8_t>(b>>8);
            scratch[pos+12]=static_cast<std::uint8_t>(score);
            scratch[pos+13]=static_cast<std::uint8_t>(score>>8);
        }
        const auto r=pixtee::original_cpu_select_best_scored_record(
            static_cast<std::uint16_t>(span),
            static_cast<std::uint16_t>(step),scratch);
        std::cout<<(r.found?1:0)<<' '<<r.original_index<<' '
                 <<r.offset_bytes<<' '<<r.original_score_word<<'\n';
    }
    return 0;
}
