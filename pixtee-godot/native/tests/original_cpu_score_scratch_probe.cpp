#include <cstdint>
#include <iostream>
#include <vector>
#include "original_cpu_score_scratch.hpp"
int main() {
    std::uint32_t span,step,seed;
    while(std::cin>>span>>step>>seed) {
        if(span>65535 || step>65535 || seed>65535)return 2;
        std::vector<std::uint8_t> buffer(
            pixtee::kOriginalCpuScoreSafeRecords*
            pixtee::kOriginalCpuScoreRecordBytes);
        for(std::size_t i=0;i<buffer.size();++i)
            buffer[i]=static_cast<std::uint8_t>(
                (seed+i*13u)&0xffu);
        const auto out=pixtee::original_cpu_clear_scoring_records(
            static_cast<std::uint16_t>(span),
            static_cast<std::uint16_t>(step),buffer);
        std::uint32_t hash=2166136261u;
        for(const auto byte:buffer) {
            hash^=byte;
            hash*=16777619u;
        }
        std::cout<<out.cleared_records<<' '
                 <<out.reset_swing_adjuster<<' '
                 <<hash<<'\n';
    }
    return 0;
}
