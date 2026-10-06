#include "sensigolf/trace.hpp"

#include <stdexcept>

namespace sensigolf {

void TraceRecorder::clear() {
    samples_.clear();
}

void TraceRecorder::capture(const BallState& state) {
    if (!samples_.empty() && state.tick <= samples_.back().tick) {
        throw std::logic_error("trace ticks must be strictly increasing");
    }

    samples_.push_back(TraceSample{
        state.tick,
        state.x_raw,
        state.y_raw,
        state.z_raw,
        state.surface_index,
        state.phase,
    });
}

} // namespace sensigolf
