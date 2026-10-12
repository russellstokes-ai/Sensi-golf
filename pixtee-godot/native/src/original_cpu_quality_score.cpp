#include "original_cpu_quality_score.hpp"
#include <stdexcept>
namespace pixtee {
const OriginalCpuPenaltyTable& original_cpu_reference_penalties() noexcept {
    // Windows 1.014 GOLFWIN.EXE SHA256 3ab09a789a...c7f1ec8;
    // first original .data image at absolute 0x41FADF.
    // Source-loader runtime table rewrite search remains separate.
    static const OriginalCpuPenaltyTable values{{
        200u,40u,30u,20u,5u,0u,0u,200u,
        200u,200u,1000u,0u,0u,0u,0u,0u,
        0u,0u,200u,200u,200u,0u,0u,0u,
        0u,0u,0u,0u,0u,0u,0u,0u
    }};
    return values;
}

std::uint16_t original_cpu_quality_score(
    std::uint16_t distance,std::uint16_t code,
    std::uint16_t green,const OriginalCpuPenaltyTable& penalties) {
    // The original x86 uses MOVSX EBX,word[record+0xA], then SHL BX
    // (low16 only) before ADD absolute table base, with no bounds check.
    // The verified native subset rejects invalid/negative indices
    // rather than accessing unrelated original process memory.
    if(code>=penalties.size())
        throw std::out_of_range(
            "CPU candidate collision code outside source-proven scoring penalty table");
    // Source CMP WORD [0x41EE7A],1; JE skips ADD AX,BX. Only mode
    // exactly one is green, not arbitrary nonzero original values.
    const std::uint16_t modifier=green==1u ? 0u : penalties[code];
    return static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(distance)+modifier);
}
} // namespace pixtee
