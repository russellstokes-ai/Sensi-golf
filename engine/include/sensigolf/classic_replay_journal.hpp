#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "sensigolf/classic_game_session.hpp"

namespace sensigolf {

// Deterministic mobile save at finished-shot boundaries: records raw player
// inputs and independently checks the *original engine* after each replay.
// It never serializes opaque mid-flight physics/PRNG pointers or course assets.
struct ClassicReplayFrame {
    ClassicHoleMetadata hole{};
    ClassicShotRequest input{};
    std::int32_t ball_x_raw = 0;
    std::int32_t ball_y_raw = 0;
    std::uint32_t distance = 0;
    std::uint32_t strokes = 0;
    HoleSessionPhase phase = HoleSessionPhase::ReadyForShot;
    bool green_mode = false;
};

class ClassicReplayJournal {
public:
    static constexpr unsigned kFormatVersion = 1;
    static constexpr std::size_t kMaxFrames = 1800;

    void append_after_shot(
        const ClassicHoleSession& completed_shot,
        const ClassicShotRequest& input);
    const std::vector<ClassicReplayFrame>& frames() const noexcept;

    // Stable text representation with format version and corruption checksum.
    std::string serialize() const;
    static ClassicReplayJournal parse(const std::string& serialized);

    // game must be a fresh, empty ClassicGameSession with the same original
    // hole plan, player slot, imported course assets, and initial PRNG seed.
    // Rejects any mismatch in resource/phase/ball/score; no synthetic drops.
    void replay_into(
        ClassicGameSession& game,
        const std::function<ClassicCourseResources(std::uint8_t)>& load_course) const;

private:
    std::vector<ClassicReplayFrame> frames_{};
};

} // namespace sensigolf
