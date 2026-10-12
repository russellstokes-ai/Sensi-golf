#include "original_cpu_score_scratch.hpp"
#include <algorithm>
#include <stdexcept>
namespace pixtee {
OriginalCpuScoreInitialization original_cpu_clear_scoring_records(
    std::uint16_t span,std::uint16_t step,
    std::vector<std::uint8_t>& bytes) {
    if(step==0)
        throw std::invalid_argument("original CPU evaluator step zero never terminates");
    std::size_t records=0;
    auto dx=span;
    for(;;) {
        if(records>=kOriginalCpuScoreSafeRecords)
            throw std::out_of_range("original CPU evaluator scratch exceeds recovered bounded capacity");
        ++records;
        // Source x86 SUB DX,[0x41FAD3] is wrapping WORD arithmetic.
        dx=static_cast<std::uint16_t>(
            static_cast<std::uint32_t>(dx)-static_cast<std::uint32_t>(step));
        // Source JNS repeats while the signed 16-bit result is nonnegative.
        if((dx&0x8000u)!=0)break;
    }
    const auto required=records*kOriginalCpuScoreRecordBytes;
    if(bytes.size()<required)
        throw std::out_of_range("source CPU record buffer too short");
    std::fill_n(bytes.begin(),required,std::uint8_t{0});
    // Source MOV DWORD [0x41F32C],0 after the record reset.
    return {records,0u};
}
} // namespace pixtee
