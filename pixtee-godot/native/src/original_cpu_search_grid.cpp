#include "original_cpu_search_grid.hpp"
#include <stdexcept>
namespace pixtee {
OriginalCpuScanGrid original_cpu_scan_grid(
    std::uint16_t mode,std::uint8_t band,
    const std::array<std::uint16_t,4>& w) {
    if(mode<4 || mode>9 || band>3)
        throw std::invalid_argument("source table indices need mode 4..9 and band 0..3");
    if(w[0]==0) // actual x86 would raise divide-error, do not emulate a crash
        throw std::invalid_argument("original CPU scan candidate divisor zero");
    const auto quotient=static_cast<std::uint16_t>(154u/w[0]);
    const auto remainder=static_cast<std::uint16_t>(154u%w[0]);
    return {
        w[0],w[1],w[2],w[3],
        quotient,
        static_cast<std::uint16_t>(154u-remainder)
    };
}
}
