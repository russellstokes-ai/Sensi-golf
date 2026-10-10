#include <cassert>
#include <cstdint>
#include <stdexcept>

#include "sensigolf/classic_model.hpp"
#include "sensigolf/trace.hpp"
#include "sensigolf/types.hpp"

namespace {

class ContractOnlyMock final : public sensigolf::IClassicModel {
public:
    void reset() override {
        state_ = {};
        active_ = false;
    }

    void begin_shot(const sensigolf::ShotInput& input) override {
        state_ = {};
        state_.phase = sensigolf::ShotPhase::Airborne;
        state_.vx_raw = input.aim_raw;
        active_ = true;
    }

    void step() override {
        if (!active_) {
            return;
        }

        ++state_.tick;
        state_.x_raw += state_.vx_raw;

        // This is deliberately a test double, not game physics.
        if (state_.tick == 3) {
            state_.phase = sensigolf::ShotPhase::Complete;
            active_ = false;
        }
    }

    const sensigolf::BallState& state() const override {
        return state_;
    }

    bool shot_active() const override {
        return active_;
    }

    std::uint32_t tick_rate_hz() const override {
        return 1;
    }

private:
    sensigolf::BallState state_{};
    bool active_ = false;
};

} // namespace

int main() {
    using namespace sensigolf;

    BallState a{};
    BallState b{};
    assert(a == b);
    b.x_raw = 1;
    assert(a != b);

    TraceRecorder recorder;
    ContractOnlyMock mock;
    mock.reset();

    ShotInput input{};
    input.aim_raw = 2;
    mock.begin_shot(input);

    while (mock.shot_active()) {
        mock.step();
        recorder.capture(mock.state());
    }

    assert(recorder.samples().size() == 3);
    assert(recorder.samples()[0].tick == 1);
    assert(recorder.samples()[0].x_raw == 2);
    assert(recorder.samples()[2].x_raw == 6);
    assert(recorder.samples()[2].phase == ShotPhase::Complete);

    bool threw = false;
    try {
        recorder.capture(mock.state());
    } catch (const std::logic_error&) {
        threw = true;
    }
    assert(threw);

    return 0;
}
