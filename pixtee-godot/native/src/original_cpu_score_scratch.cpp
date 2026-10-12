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
namespace {
int signed16(std::uint16_t x) noexcept {
    return x<0x8000u ? static_cast<int>(x) :
           static_cast<int>(x)-0x10000;
}
std::uint16_t word(const std::vector<std::uint8_t>& bytes,std::size_t pos) {
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(bytes[pos])|
        (static_cast<std::uint16_t>(bytes[pos+1])<<8));
}
}
OriginalCpuScoreFirstEligible original_cpu_first_eligible_score(
    std::uint16_t span,std::uint16_t step,
    const std::vector<std::uint8_t>& bytes) {
    if(step==0)
        throw std::invalid_argument("unbounded original evaluator stride zero");
    std::uint16_t cx=0,bx=0,dx=0;
    std::size_t index=0;
    for(;;) {
        if(index>=kOriginalCpuScoreSafeRecords)
            throw std::out_of_range("original first-eligible scan exceeds bounded score scratch");
        const auto off=index*kOriginalCpuScoreRecordBytes;
        if(off>bytes.size() || bytes.size()-off<4u)
            throw std::out_of_range("original evaluator candidate record truncated");
        // Source 0x40CA67 checks signed WORD [EBP+2] first, then
        // signed WORD [EBP+0]. Either negative rejects this record.
        if(signed16(word(bytes,off+2u))>=0 &&
           signed16(word(bytes,off))>=0)
            return {true,index,cx,bx,dx};
        ++index;
        bx=static_cast<std::uint16_t>(bx+1u);
        dx=static_cast<std::uint16_t>(dx+1u);
        cx=static_cast<std::uint16_t>(
            static_cast<std::uint32_t>(cx)+static_cast<std::uint32_t>(step));
        // Original CMP CX,[span]; JL is SIGNED 16-bit, not unsigned.
        if(signed16(cx)>=signed16(span))
            return {false,index,cx,bx,dx};
    }
}
OriginalCpuBestScoredRecord original_cpu_select_best_scored_record(
    std::uint16_t span,std::uint16_t step,
    const std::vector<std::uint8_t>& bytes) {
    const auto first=original_cpu_first_eligible_score(span,step,bytes);
    if(!first.found)
        return {};
    std::size_t index=first.original_index;
    std::uint16_t cx=first.scanned_cx;
    std::uint16_t dx=first.scanned_dx;
    std::uint16_t selected_index=first.scanned_bx;
    const auto initial_offset=index*kOriginalCpuScoreRecordBytes;
    if(initial_offset>bytes.size() ||
       bytes.size()-initial_offset<kOriginalCpuScoreRecordBytes)
        throw std::out_of_range("source CPU score WORD not in candidate record");
    auto best_score=word(bytes,initial_offset+12u);
    for(;;) {
        if(index>=kOriginalCpuScoreSafeRecords)
            throw std::out_of_range("source candidate scoring exhausted verified scratch");
        const auto off=index*kOriginalCpuScoreRecordBytes;
        if(off>bytes.size() || bytes.size()-off<kOriginalCpuScoreRecordBytes)
            throw std::out_of_range("original CPU candidate score record truncated");
        if(signed16(word(bytes,off))>=0 &&
           signed16(word(bytes,off+2u))>=0 &&
           signed16(best_score)>signed16(word(bytes,off+12u))) {
            best_score=word(bytes,off+12u);
            selected_index=dx;
        }
        ++index;
        dx=static_cast<std::uint16_t>(dx+1u);
        cx=static_cast<std::uint16_t>(
            static_cast<std::uint32_t>(cx)+static_cast<std::uint32_t>(step));
        // Source signed JL after CMP CX,[0x41FA0F].
        if(signed16(cx)>=signed16(span))
            break;
    }
    return {true,selected_index,
            static_cast<std::size_t>(selected_index)*
                kOriginalCpuScoreRecordBytes,best_score};
}
} // namespace pixtee
