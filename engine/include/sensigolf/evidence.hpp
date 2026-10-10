#pragma once

#include <cstdint>
#include <string>

namespace sensigolf {

enum class EvidenceKind : std::uint8_t {
    ExecutableOffset = 0,
    DataFileOffset,
    BlackBoxTrace,
    ManualConstraint,
};

struct EvidenceAnchor {
    EvidenceKind kind = EvidenceKind::ExecutableOffset;

    // SHA-256 of the exact source build/file when applicable.
    std::string source_sha256;

    // File or trace identity, e.g. GOLFWIN.EXE or a golden-master trace id.
    std::string source_name;

    // Raw file offset when applicable; zero is not implicitly meaningful.
    std::uint64_t offset = 0;

    // Human-readable explanation of what has actually been observed.
    std::string observation;
};

} // namespace sensigolf
