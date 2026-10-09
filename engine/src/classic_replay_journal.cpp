#include "sensigolf/classic_replay_journal.hpp"

#include <cstdint>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace sensigolf {
namespace {

std::uint64_t fnv64(const std::string& input) {
    std::uint64_t hash = 14695981039346656037ULL;
    for (const unsigned char byte : input) {
        hash ^= byte;
        hash *= 1099511628211ULL;
    }
    return hash;
}

void validate_frame(const ClassicReplayFrame& f) {
    if (f.hole.hole_index >= 18 || f.hole.par < 1 || f.hole.par > 9
        || f.hole.resource_id >= 100
        || f.input.aim_raw >= 4096 || f.input.power_tick < 0
        || f.input.power_tick > 105 || f.input.accuracy_tick < 0
        || f.input.accuracy_tick > 105 || f.input.club_index > 12
        || f.strokes == 0 || f.strokes > 10000
        || (f.phase != HoleSessionPhase::ReadyForShot
            && f.phase != HoleSessionPhase::HoleScored)) {
        throw std::invalid_argument("invalid original-golf replay frame");
    }
}

void validate_next(
    const std::vector<ClassicReplayFrame>& previous,
    const ClassicReplayFrame& next) {
    validate_frame(next);
    if (previous.empty()) {
        if (next.hole.hole_index != 0 || next.strokes != 1) {
            throw std::invalid_argument("replay must start at first original stroke");
        }
        return;
    }
    const auto& last = previous.back();
    if (next.hole.hole_index == last.hole.hole_index) {
        if (last.phase != HoleSessionPhase::ReadyForShot
            || next.hole.resource_id != last.hole.resource_id
            || next.hole.par != last.hole.par
            || next.strokes != last.strokes + 1u) {
            throw std::invalid_argument("replay has duplicate or out-of-order stroke");
        }
    } else if (next.hole.hole_index == last.hole.hole_index + 1u) {
        if (last.phase != HoleSessionPhase::HoleScored
            || next.strokes != 1) {
            throw std::invalid_argument("replay crossed an unscored hole");
        }
    } else {
        throw std::invalid_argument("replay skipped original hole order");
    }
}

void execute_recovered_shot(
    ClassicHoleSession& hole, const ClassicShotRequest& request) {
    if (hole.phase() != HoleSessionPhase::ReadyForShot) {
        throw std::runtime_error("replayed original hole is not ready");
    }
    hole.begin_shot(request);
    int ticks = 0;
    while (hole.phase() == HoleSessionPhase::ShotActive && ticks++ < 4096) {
        hole.step();
    }
    if (hole.phase() == HoleSessionPhase::ShotActive) {
        throw std::runtime_error("replay exceeded recovered shot tick budget");
    }
    if (hole.phase() == HoleSessionPhase::HazardStopped) {
        int pause = 0;
        while (hole.phase() == HoleSessionPhase::HazardStopped && pause++ < 104) {
            hole.step();
        }
        if (hole.phase() == HoleSessionPhase::HazardRecovered) {
            hole.acknowledge_hazard_recovery();
        }
    }
    if (hole.phase() == HoleSessionPhase::SpecialGreenStopped) {
        throw std::runtime_error(
            "cannot replay original code-9/10 event-11 without verified continuation");
    }
    if (hole.phase() != HoleSessionPhase::ReadyForShot
        && hole.phase() != HoleSessionPhase::CupTerminal) {
        throw std::runtime_error("replay reached unresolved original rule");
    }
    if (hole.distance_to_hole() == 0) {
        hole.step();
    }
}

} // namespace

void ClassicReplayJournal::append_after_shot(
    const ClassicHoleSession& hole, const ClassicShotRequest& input) {
    if (!hole.hole_metadata()) {
        throw std::invalid_argument("replay requires original hole metadata");
    }
    if (frames_.size() >= kMaxFrames) {
        throw std::length_error("original replay exceeded bounded journal capacity");
    }
    ClassicReplayFrame frame{};
    frame.hole = *hole.hole_metadata();
    frame.input = input;
    frame.ball_x_raw = hole.ball_x_raw();
    frame.ball_y_raw = hole.ball_y_raw();
    frame.distance = hole.distance_to_hole();
    frame.strokes = hole.strokes();
    frame.phase = hole.phase();
    frame.green_mode = hole.green_mode();
    validate_next(frames_, frame);
    frames_.push_back(frame);
}

const std::vector<ClassicReplayFrame>&
ClassicReplayJournal::frames() const noexcept {
    return frames_;
}

std::string ClassicReplayJournal::serialize() const {
    std::ostringstream out;
    out << "SENSIGOLF_REPLAY " << kFormatVersion << "\n"
        << frames_.size() << "\n";
    for (const auto& f : frames_) {
        out << f.hole.hole_index << ' ' << f.hole.resource_id << ' '
            << f.hole.par << ' ' << f.input.aim_raw << ' '
            << f.input.power_tick << ' ' << f.input.accuracy_tick << ' '
            << f.input.club_index << ' ' << f.ball_x_raw << ' '
            << f.ball_y_raw << ' ' << f.distance << ' ' << f.strokes
            << ' ' << static_cast<unsigned>(f.phase) << ' '
            << (f.green_mode ? 1 : 0) << "\n";
    }
    const auto payload = out.str();
    return payload + "FNV64 " + std::to_string(fnv64(payload)) + "\n";
}

ClassicReplayJournal
ClassicReplayJournal::parse(const std::string& serialized) {
    if (serialized.size() > 300000u || serialized.empty()
        || serialized.back() != '\n') {
        throw std::invalid_argument("replay serialization length/terminator invalid");
    }
    const auto signature_at = serialized.rfind("FNV64 ");
    if (signature_at == std::string::npos
        || (signature_at != 0 && serialized[signature_at - 1] != '\n')) {
        throw std::invalid_argument("replay integrity marker missing");
    }
    const auto signature = serialized.substr(signature_at + 6);
    if (signature.empty() || signature.back() != '\n') {
        throw std::invalid_argument("replay integrity signature incomplete");
    }
    const auto expected = std::to_string(fnv64(
        serialized.substr(0, signature_at))) + "\n";
    if (signature != expected) {
        throw std::invalid_argument("replay corrupted or checksum mismatch");
    }

    std::istringstream input(serialized.substr(0, signature_at));
    std::string magic;
    unsigned version = 0;
    std::size_t count = 0;
    if (!(input >> magic >> version >> count)
        || magic != "SENSIGOLF_REPLAY" || version != kFormatVersion
        || count > kMaxFrames) {
        throw std::invalid_argument("unrecognized replay format/version/count");
    }

    ClassicReplayJournal result{};
    for (std::size_t i = 0; i < count; ++i) {
        std::int64_t fields[13]{};
        for (auto& field : fields) {
            if (!(input >> field)) {
                throw std::invalid_argument("truncated original replay record");
            }
        }
        const auto& v = fields;
        if (v[0] < 0 || v[0] > 17 || v[1] < 0 || v[1] > 99
            || v[2] < 1 || v[2] > 9 || v[3] < 0 || v[3] > 4095
            || v[4] < 0 || v[4] > 105 || v[5] < 0 || v[5] > 105
            || v[6] < 0 || v[6] > 12
            || v[7] < std::numeric_limits<std::int32_t>::min()
            || v[7] > std::numeric_limits<std::int32_t>::max()
            || v[8] < std::numeric_limits<std::int32_t>::min()
            || v[8] > std::numeric_limits<std::int32_t>::max()
            || v[9] < 0 || v[9] > std::numeric_limits<std::uint32_t>::max()
            || v[10] < 1 || v[10] > 10000
            || (v[11] != static_cast<int>(HoleSessionPhase::ReadyForShot)
                && v[11] != static_cast<int>(HoleSessionPhase::HoleScored))
            || (v[12] != 0 && v[12] != 1)) {
            throw std::invalid_argument("out-of-range original replay value");
        }
        ClassicReplayFrame frame{};
        frame.hole = ClassicHoleMetadata{
            static_cast<std::uint16_t>(v[0]),
            static_cast<std::uint16_t>(v[2]),
            static_cast<std::uint16_t>(v[1])};
        frame.input = ClassicShotRequest{
            static_cast<RawScalar>(v[3]), static_cast<std::int32_t>(v[4]),
            static_cast<std::int32_t>(v[5]), static_cast<std::uint16_t>(v[6])};
        frame.ball_x_raw = static_cast<std::int32_t>(v[7]);
        frame.ball_y_raw = static_cast<std::int32_t>(v[8]);
        frame.distance = static_cast<std::uint32_t>(v[9]);
        frame.strokes = static_cast<std::uint32_t>(v[10]);
        frame.phase = static_cast<HoleSessionPhase>(v[11]);
        frame.green_mode = v[12] == 1;
        validate_next(result.frames_, frame);
        result.frames_.push_back(frame);
    }
    std::string extra;
    if (input >> extra) {
        throw std::invalid_argument("unexpected extra replay records");
    }
    return result;
}

void ClassicReplayJournal::replay_into(
    ClassicGameSession& game,
    const std::function<ClassicCourseResources(std::uint8_t)>& load_course) const {
    if (game.has_active_hole() || game.round_state().holes_completed != 0
        || game.round_state().current_hole_index != 0) {
        throw std::invalid_argument(
            "mobile replay must restore into a fresh classic game session");
    }
    for (const auto& frame : frames_) {
        const auto requested = game.resource_request();
        if (!requested || requested->round_index != frame.hole.hole_index
            || requested->resource_id != frame.hole.resource_id
            || requested->par != frame.hole.par) {
            throw std::runtime_error("save does not match selected original course plan");
        }
        if (!game.has_active_hole()) {
            game.load_current_hole(
                requested->resource_id, load_course(requested->resource_id));
        }
        auto& hole = game.active_hole();
        execute_recovered_shot(hole, frame.input);
        if (hole.phase() != frame.phase
            || hole.ball_x_raw() != frame.ball_x_raw
            || hole.ball_y_raw() != frame.ball_y_raw
            || hole.distance_to_hole() != frame.distance
            || hole.strokes() != frame.strokes
            || hole.green_mode() != frame.green_mode) {
            throw std::runtime_error(
                "saved original shot diverges from authoritative replay");
        }
        if (hole.phase() == HoleSessionPhase::HoleScored) {
            game.commit_scored_hole();
        }
    }
}

} // namespace sensigolf
