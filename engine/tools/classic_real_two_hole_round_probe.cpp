#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include "sensigolf/classic_game_session.hpp"

namespace {

std::vector<std::uint8_t> read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open original resource: " + path);
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>());
}

std::string resource_filename(
    const std::string& root, const char* prefix,
    std::uint8_t id, const char* extension) {
    const auto number = static_cast<unsigned>(id);
    return root + "/" + prefix + (number < 10u ? "0" : "")
        + std::to_string(number) + "." + extension;
}

sensigolf::ClassicCourseResources load_original_course(
    const std::string& root, std::uint8_t id,
    const std::vector<std::uint8_t>& descriptors,
    const std::vector<std::uint8_t>& selection) {
    return sensigolf::ClassicCourseResources(
        read_file(resource_filename(root, "MAPM", id, "MAP")),
        read_file(resource_filename(root, "MAPM", id, "SPT")),
        descriptors, selection,
        read_file(resource_filename(root, "MAPS", id, "MAP")));
}

void play_shot(
    sensigolf::ClassicHoleSession& hole,
    const sensigolf::ClassicShotRequest& request) {
    if (hole.phase() != sensigolf::HoleSessionPhase::ReadyForShot) {
        throw std::runtime_error("not ready for recorded original-physics shot");
    }
    hole.begin_shot(request);
    for (int tick = 0;
         tick < 4096 && hole.phase() == sensigolf::HoleSessionPhase::ShotActive;
         ++tick) {
        hole.step();
    }
    if (hole.phase() == sensigolf::HoleSessionPhase::ShotActive) {
        throw std::runtime_error("shot exceeded deterministic tick budget");
    }
    if (hole.phase() == sensigolf::HoleSessionPhase::HazardStopped
        || hole.phase() == sensigolf::HoleSessionPhase::SpecialGreenStopped
        || hole.phase() == sensigolf::HoleSessionPhase::UnsupportedTerrain) {
        throw std::runtime_error(
            "recorded route entered unresolved or unsupported terminal");
    }
    if (hole.distance_to_hole() == 0
        && (hole.phase() == sensigolf::HoleSessionPhase::CupTerminal
            || hole.phase() == sensigolf::HoleSessionPhase::ReadyForShot)) {
        hole.step();
    }
}

template <std::size_t N>
void complete_original_hole(
    sensigolf::ClassicGameSession& game,
    std::uint8_t expected_id,
    std::size_t expected_index,
    const std::array<sensigolf::ClassicShotRequest, N>& shots) {
    const auto meta = game.active_hole().hole_metadata();
    if (!meta || meta->resource_id != expected_id
        || meta->hole_index != expected_index) {
        throw std::runtime_error("original hole metadata or sequence mismatch");
    }
    for (std::size_t i = 0; i < N; ++i) {
        play_shot(game.active_hole(), shots[i]);
        if (i + 1 < N && game.active_hole().phase()
            != sensigolf::HoleSessionPhase::ReadyForShot) {
            throw std::runtime_error("original hole scored before final recorded shot");
        }
    }
    if (game.active_hole().phase() != sensigolf::HoleSessionPhase::HoleScored
        || game.active_hole().strokes() != N) {
        throw std::runtime_error("original hole did not score on recorded route");
    }
    game.commit_scored_hole();
    if (game.has_active_hole() || game.round_state().holes_completed != expected_index + 1) {
        throw std::runtime_error("scored-hole commit did not advance the same round");
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 9) {
        std::cerr << "usage: sensigolf_real_two_hole_round_probe "
                  << "<original-epf-root> <id1> <par1> <id2> <par2> "
                  << "<id3> <par3> <player-slot>\n";
        return 2;
    }
    try {
        const std::string root = argv[1];
        const auto first = static_cast<std::uint8_t>(std::stoul(argv[2]));
        const auto first_par = static_cast<std::uint8_t>(std::stoul(argv[3]));
        const auto second = static_cast<std::uint8_t>(std::stoul(argv[4]));
        const auto second_par = static_cast<std::uint8_t>(std::stoul(argv[5]));
        const auto third = static_cast<std::uint8_t>(std::stoul(argv[6]));
        const auto third_par = static_cast<std::uint8_t>(std::stoul(argv[7]));
        const auto player_slot = static_cast<std::size_t>(std::stoul(argv[8]));
        if (first != 42 || second != 50 || third != 58
            || first_par != 4 || second_par != 4 || third_par != 3
            || player_slot != 0) {
            throw std::runtime_error(
                "fixture must use recovered Windows v1.014 slot-0 sequence 42/50/58");
        }

        const auto descriptors = read_file(root + "/MAPI01.RAW");
        const auto selection = read_file(root + "/MAPI02.RAW");
        std::array<std::uint8_t, sensigolf::kClassicRoundHoleCount> order{};
        order.fill(third);
        order[0] = first;
        order[1] = second;
        order[2] = third;
        std::array<std::uint8_t, sensigolf::kClassicParTableSize> pars{};
        pars[first] = first_par;
        pars[second] = second_par;
        pars[third] = third_par;
        sensigolf::ClassicGameSession game(
            sensigolf::ClassicHolePlan(order, pars), player_slot);

        auto expected_request = [&](std::uint8_t id, std::uint16_t index,
                                    std::uint8_t par) {
            const auto request = game.resource_request();
            if (!request || request->resource_id != id
                || request->round_index != index || request->par != par) {
                throw std::runtime_error("same game session requested wrong next hole");
            }
        };
        expected_request(first, 0u, first_par);
        game.load_current_hole(first, load_original_course(
            root, first, descriptors, selection));

        // Two independently recovered original-physics paths are replayed
        // here in ONE authoritative game session. No independent restart.
        constexpr std::array<sensigolf::ClassicShotRequest, 3> first_shots{{
            {1882, 105, 63, 0},
            {1882, 95, 63, 2},
            {1756, 5, 63, 3},
        }};
        complete_original_hole(game, first, 0u, first_shots);
        expected_request(second, 1u, second_par);
        if (game.round_state().total_strokes != 3u
            || game.round_state().cumulative_par != 4u
            || game.round_state().relative_to_par != 1) {
            throw std::runtime_error("first-hole scorecard did not persist");
        }

        game.load_current_hole(second, load_original_course(
            root, second, descriptors, selection));
        constexpr std::array<sensigolf::ClassicShotRequest, 4> second_shots{{
            {2335, 105, 63, 1},
            {1893, 105, 63, 1},
            {2908, 95, 63, 1},
            {2800, 5, 63, 3},
        }};
        complete_original_hole(game, second, 1u, second_shots);
        expected_request(third, 2u, third_par);

        const auto& score = game.round_state();
        if (score.holes_completed != 2u || score.current_hole_index != 2u
            || score.total_strokes != 7u || score.cumulative_par != 8u
            || score.relative_to_par != 1 || score.round_complete) {
            throw std::runtime_error("two-hole continuous scorecard is wrong");
        }

        auto third_course = load_original_course(
            root, third, descriptors, selection);
        const auto tee = third_course.player_start(player_slot);
        const auto cup = third_course.hole_position();
        game.load_current_hole(third, std::move(third_course));
        const auto meta = game.active_hole().hole_metadata();
        if (!meta || meta->hole_index != 2u || meta->resource_id != third
            || game.active_hole().phase() != sensigolf::HoleSessionPhase::ReadyForShot
            || game.active_hole().ball_x_raw() != tee.x_raw()
            || game.active_hole().ball_y_raw() != tee.y_raw()
            || tee.x != 370 || tee.y != 383 || cup.x != 138 || cup.y != 96) {
            throw std::runtime_error("third original course failed to load at its actual tee");
        }

        std::cout << "{\"resources\":["
                  << static_cast<unsigned>(first) << ","
                  << static_cast<unsigned>(second) << ","
                  << static_cast<unsigned>(third)
                  << "],\"completed_strokes\":[3,4],\"holes_completed\":"
                  << score.holes_completed << ",\"total_strokes\":"
                  << score.total_strokes << ",\"cumulative_par\":"
                  << score.cumulative_par << ",\"relative_to_par\":"
                  << score.relative_to_par
                  << ",\"next_loaded\":true,\"next_tee\":["
                  << tee.x << "," << tee.y << "],\"next_cup\":["
                  << cup.x << "," << cup.y << "]}\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
