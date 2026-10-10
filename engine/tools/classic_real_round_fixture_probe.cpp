// Data-driven real-original-course session replay; no gameplay substitutes.
// Fixture contains only recovered numeric inputs, not commercial assets.
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "sensigolf/classic_game_session.hpp"

namespace {

struct HoleFixture {
    std::uint8_t id = 0;
    std::uint8_t par = 0;
    int tee_x = 0;
    int tee_y = 0;
    int cup_x = 0;
    int cup_y = 0;
    std::vector<sensigolf::ClassicShotRequest> shots;
};

std::vector<std::uint8_t> read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("original course file missing: " + path);
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

std::string original_path(
    const std::string& root, const char* prefix,
    std::uint8_t id, const char* extension) {
    const auto n = static_cast<unsigned>(id);
    return root + "/" + prefix + (n < 10u ? "0" : "")
        + std::to_string(n) + "." + extension;
}

sensigolf::ClassicCourseResources load_course(
    const std::string& root, std::uint8_t id,
    const std::vector<std::uint8_t>& desc,
    const std::vector<std::uint8_t>& select) {
    return sensigolf::ClassicCourseResources(
        read_file(original_path(root, "MAPM", id, "MAP")),
        read_file(original_path(root, "MAPM", id, "SPT")),
        desc, select,
        read_file(original_path(root, "MAPS", id, "MAP")));
}

std::vector<HoleFixture> parse_fixture(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("fixture missing: " + filename);
    std::vector<HoleFixture> rows;
    std::string line;
    for (int line_number = 1; std::getline(in, line); ++line_number) {
        if (line.empty() || line.find_first_not_of(" \t\r") == std::string::npos
            || line.find_first_not_of(" \t") == line.find('#')) {
            continue;
        }
        std::istringstream fields(line);
        int id = 0, par = 0, nshots = 0;
        HoleFixture hole{};
        if (!(fields >> id >> par >> hole.tee_x >> hole.tee_y
              >> hole.cup_x >> hole.cup_y >> nshots)
            || id < 0 || id >= sensigolf::kClassicParTableSize
            || par < 1 || par > 9 || nshots < 0 || nshots > 100) {
            throw std::runtime_error(
                "invalid fixture header at line " + std::to_string(line_number));
        }
        hole.id = static_cast<std::uint8_t>(id);
        hole.par = static_cast<std::uint8_t>(par);
        for (int i = 0; i < nshots; ++i) {
            int aim = 0, power = 0, accuracy = 0, club = 0;
            if (!(fields >> aim >> power >> accuracy >> club)
                || aim < 0 || aim >= 4096 || power < 0 || power > 105
                || accuracy < 0 || accuracy > 105 || club < 0 || club > 12) {
                throw std::runtime_error(
                    "invalid recorded original shot at line "
                    + std::to_string(line_number));
            }
            hole.shots.push_back(sensigolf::ClassicShotRequest{
                static_cast<sensigolf::RawScalar>(aim),
                power, accuracy, static_cast<std::uint16_t>(club)});
        }
        std::string trailing;
        if (fields >> trailing) {
            throw std::runtime_error(
                "unexpected extra fixture data at line "
                + std::to_string(line_number));
        }
        rows.push_back(std::move(hole));
    }
    if (rows.size() < 2 || rows.size() > sensigolf::kClassicRoundHoleCount) {
        throw std::runtime_error("fixture must cover 2 to 18 original holes");
    }
    for (std::size_t i = 0; i + 1 < rows.size(); ++i) {
        if (rows[i].shots.empty()) {
            throw std::runtime_error("non-final fixture hole has zero shots");
        }
    }
    if (rows.size() < sensigolf::kClassicRoundHoleCount
        && !rows.back().shots.empty()) {
        throw std::runtime_error(
            "partial round fixture must end with one unplayed next course");
    }
    return rows;
}

void play_original_shot(
    sensigolf::ClassicHoleSession& hole,
    const sensigolf::ClassicShotRequest& request) {
    if (hole.phase() != sensigolf::HoleSessionPhase::ReadyForShot) {
        throw std::runtime_error("recorded shot began from non-playable state");
    }
    hole.begin_shot(request);
    for (int tick = 0; tick < 4096
         && hole.phase() == sensigolf::HoleSessionPhase::ShotActive; ++tick) {
        hole.step();
    }
    if (hole.phase() == sensigolf::HoleSessionPhase::ShotActive) {
        throw std::runtime_error("recorded shot exceeded tick budget");
    }
    if (hole.phase() != sensigolf::HoleSessionPhase::ReadyForShot
        && hole.phase() != sensigolf::HoleSessionPhase::CupTerminal) {
        throw std::runtime_error("recorded shot needs an unresolved terminal rule");
    }
    if (hole.distance_to_hole() == 0) {
        hole.step();
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "usage: sensigolf_real_round_fixture_probe "
                  << "<original-epf-directory> <numeric-fixture-file> <player-slot>\n";
        return 2;
    }
    try {
        const std::string root = argv[1];
        const auto rows = parse_fixture(argv[2]);
        const auto slot = static_cast<std::size_t>(std::stoul(argv[3]));
        if (slot != 0u) {
            throw std::runtime_error("current original slot-0 fixture requires player 0");
        }
        std::array<std::uint8_t, sensigolf::kClassicRoundHoleCount> order{};
        order.fill(rows.back().id);
        std::array<std::uint8_t, sensigolf::kClassicParTableSize> pars{};
        for (std::size_t i = 0; i < rows.size(); ++i) {
            order[i] = rows[i].id;
            pars[rows[i].id] = rows[i].par;
        }
        sensigolf::ClassicGameSession game(
            sensigolf::ClassicHolePlan(order, pars), slot);
        const auto desc = read_file(root + "/MAPI01.RAW");
        const auto select = read_file(root + "/MAPI02.RAW");
        std::uint64_t trajectory_hash = 14695981039346656037ULL;
        auto mix_state = [&trajectory_hash](std::uint64_t value) {
            // FNV-1a over fixed byte order: stable across host endianness.
            for (unsigned i = 0; i < 8; ++i) {
                trajectory_hash ^= (value >> (i * 8u)) & 0xFFu;
                trajectory_hash *= 1099511628211ULL;
            }
        };
        std::uint32_t completed_strokes = 0;
        std::uint32_t completed_par = 0;
        std::size_t completed = 0;
        bool next_loaded = false;

        for (std::size_t index = 0; index < rows.size(); ++index) {
            const auto& row = rows[index];
            const auto request = game.resource_request();
            if (!request || request->round_index != index
                || request->resource_id != row.id || request->par != row.par) {
                throw std::runtime_error("incorrect original hole order/resource request");
            }
            auto course = load_course(root, row.id, desc, select);
            const auto tee = course.player_start(slot);
            const auto cup = course.hole_position();
            if (tee.x != row.tee_x || tee.y != row.tee_y
                || cup.x != row.cup_x || cup.y != row.cup_y) {
                throw std::runtime_error("original SPT tee/cup differs from verified fixture");
            }
            game.load_current_hole(row.id, std::move(course));
            const auto metadata = game.active_hole().hole_metadata();
            if (!metadata || metadata->hole_index != index
                || metadata->resource_id != row.id
                || game.active_hole().phase() != sensigolf::HoleSessionPhase::ReadyForShot
                || game.active_hole().ball_x_raw() != tee.x_raw()
                || game.active_hole().ball_y_raw() != tee.y_raw()) {
                throw std::runtime_error("original course did not activate correctly");
            }
            if (row.shots.empty()) {
                if (index + 1 != rows.size() || index == 0) {
                    throw std::runtime_error("unplayed fixture course not last");
                }
                next_loaded = true;
                break;
            }

            for (std::size_t i = 0; i < row.shots.size(); ++i) {
                play_original_shot(game.active_hole(), row.shots[i]);
                const auto& state = game.active_hole();
                mix_state(static_cast<std::uint64_t>(index));
                mix_state(static_cast<std::uint64_t>(i));
                mix_state(static_cast<std::uint32_t>(state.ball_x_raw()));
                mix_state(static_cast<std::uint32_t>(state.ball_y_raw()));
                mix_state(state.distance_to_hole());
                mix_state(state.strokes());
                mix_state(static_cast<std::uint64_t>(state.green_mode()));
                mix_state(static_cast<std::uint64_t>(state.phase()));
                if (i + 1 != row.shots.size()
                    && game.active_hole().phase()
                        != sensigolf::HoleSessionPhase::ReadyForShot) {
                    throw std::runtime_error("hole scored before last fixture shot");
                }
            }
            if (game.active_hole().phase() != sensigolf::HoleSessionPhase::HoleScored
                || game.active_hole().strokes() != row.shots.size()) {
                throw std::runtime_error(
                    "original hole failed to score on recorded shot path");
            }
            game.commit_scored_hole();
            ++completed;
            completed_strokes += static_cast<std::uint32_t>(row.shots.size());
            completed_par += row.par;
            const auto& score = game.round_state();
            if (score.holes_completed != completed
                || score.current_hole_index != completed
                || score.total_strokes != completed_strokes
                || score.cumulative_par != completed_par
                || score.relative_to_par
                    != static_cast<std::int32_t>(completed_par)
                       - static_cast<std::int32_t>(completed_strokes)
                || score.round_complete != (completed == 18u)) {
                throw std::runtime_error(
                    "cross-hole recovered scorecard/round state regressed");
            }
        }
        const auto& score = game.round_state();
        if (!next_loaded && !score.round_complete) {
            throw std::runtime_error(
                "fixture ended without a loaded next course or full round finish");
        }
        std::cout << "{\"resources\":[";
        for (std::size_t i = 0; i < rows.size(); ++i) {
            if (i) std::cout << ",";
            std::cout << static_cast<unsigned>(rows[i].id);
        }
        std::cout << "],\"completed_strokes\":[";
        for (std::size_t i = 0; i < completed; ++i) {
            if (i) std::cout << ",";
            std::cout << rows[i].shots.size();
        }
        std::cout << "],\"trajectory_hash\":" << trajectory_hash
                  << ",\"holes_completed\":" << score.holes_completed
                  << ",\"total_strokes\":" << score.total_strokes
                  << ",\"cumulative_par\":" << score.cumulative_par
                  << ",\"relative_to_par\":" << score.relative_to_par
                  << ",\"next_loaded\":" << (next_loaded ? "true" : "false")
                  << ",\"round_complete\":" << (score.round_complete ? "true" : "false");
        if (next_loaded) {
            std::cout << ",\"next_tee\":[" << rows.back().tee_x
                      << "," << rows.back().tee_y
                      << "],\"next_cup\":[" << rows.back().cup_x
                      << "," << rows.back().cup_y << "]";
        }
        std::cout << "}\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
