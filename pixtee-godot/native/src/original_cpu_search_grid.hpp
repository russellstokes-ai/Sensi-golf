#pragma once
#include <array>
#include <cstdint>
namespace pixtee {
// Original Windows v1.014 AI scan setup 0x40C6B2..0x40C71B.
// Table indexed by (player_mode-4)*8 + course_band*48, six modes per
// course band. Caller passes authentic original 4-word table entry;
// no copyrighted original binary table is embedded in this implementation.
struct OriginalCpuScanGrid {
    std::uint16_t table_word_0=0;   // 0x41FAD1: candidate count
    std::uint16_t table_word_1=0;   // 0x41FAD5: first angular extent
    std::uint16_t table_word_2=0;   // 0x41FAD9: second angular extent
    std::uint16_t table_word_3=0;   // 0x41FAD3: original fourth word
    std::uint16_t spacing=0;        // 0x420533
    std::uint16_t initial_remainder=0; // 0x420531
};
// Authenticated 24-row Windows v1.014 numerical table, rights review pending.
OriginalCpuScanGrid original_cpu_scan_grid_from_original_table(
    std::uint16_t mode, std::uint8_t band);
OriginalCpuScanGrid original_cpu_scan_grid(
    std::uint16_t mode, std::uint8_t band,
    const std::array<std::uint16_t,4>& original_table_words);
}
