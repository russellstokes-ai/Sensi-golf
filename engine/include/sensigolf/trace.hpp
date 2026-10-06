#pragma once

#include <vector>

#include "sensigolf/types.hpp"

namespace sensigolf {

struct TraceSample {
    Tick tick = 0;
    RawScalar x_raw = 0;
    RawScalar y_raw = 0;
    RawScalar z_raw = 0;
    std::uint16_t surface_index = 0;
    ShotPhase phase = ShotPhase::Idle;
};

class TraceRecorder {
public:
    void clear();
    void capture(const BallState& state);

    const std::vector<TraceSample>& samples() const noexcept {
        return samples_;
    }

private:
    std::vector<TraceSample> samples_;
};

} // namespace sensigolf
