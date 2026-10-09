#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "sensigolf/classic_game_session.hpp"

namespace {

std::vector<std::uint8_t> read_file(const char* path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error(std::string("cannot open ") + path);
    }
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>());
}

std::uint16_t direction_to(
    std::int32_t x0,
    std::int32_t y0,
    std::int32_t x1,
    std::int32_t y1) {
    const auto dx = static_cast<double>(
        static_cast<std::int64_t>(x1) - x0);
    const auto dy = static_cast<double>(
        static_cast<std::int64_t>(y1) - y0);
    auto angle = std::atan2(dx, dy);
    constexpr double pi = 3.14159265358979323846;
    if (angle < 0.0) {
        angle += 2.0 * pi;
    }
    const auto scaled = static_cast<long>(
        std::llround(angle * 4096.0 / (2.0 * pi)));
    return static_cast<std::uint16_t>(scaled) & 0x0FFFu;
}

struct ShotChoice {
    sensigolf::ClassicShotRequest request{};
    std::uint32_t resulting_distance = std::numeric_limits<std::uint32_t>::max();
    sensigolf::HoleSessionPhase phase = sensigolf::HoleSessionPhase::UnsupportedTerrain;
    bool valid = false;
};

void run_until_terminal(sensigolf::ClassicHoleSession& hole) {
    int ticks = 0;
    while (hole.phase() == sensigolf::HoleSessionPhase::ShotActive
           && ticks++ < 4096) {
        hole.step();
    }
    if (ticks >= 4096) {
        throw std::runtime_error("shot exceeded solver tick budget");
    }
}

void finish_replay_gate(sensigolf::ClassicHoleSession& hole) {
    if (hole.phase() == sensigolf::HoleSessionPhase::SpecialGreenStopped) {
        while (hole.special_green_pause_remaining() > 0) {
            hole.step();
        }
        hole.acknowledge_special_green_stop();
    } else if (hole.phase() == sensigolf::HoleSessionPhase::HazardStopped) {
        while (hole.phase() == sensigolf::HoleSessionPhase::HazardStopped) {
            hole.step();
        }
        if (hole.phase() == sensigolf::HoleSessionPhase::HazardRecovered) {
            hole.acknowledge_hazard_recovery();
        }
    }
}

ShotChoice choose_shot(
    const sensigolf::ClassicCourseResources& course,
    const sensigolf::ClassicHoleSession& current) {
    const auto surface = current.current_surface();
    const auto cup = current.hole_position();

    std::int32_t target_x =
        static_cast<std::int32_t>(
            static_cast<std::uint32_t>(cup.x) << 16);
    std::int32_t target_y =
        static_cast<std::int32_t>(
            static_cast<std::uint32_t>(cup.y) << 16);

    if (current.green_mode()) {
        const auto region = course.green_region();
        if (!region) {
            throw std::runtime_error(
                "green-mode solver state missing recovered green region");
        }
        target_x = static_cast<std::int32_t>(
            (static_cast<std::uint32_t>(cup.x - region->origin_x) << 16)
            << 1);
        target_y = static_cast<std::int32_t>(
            (static_cast<std::uint32_t>(cup.y - region->origin_y) << 16)
            << 1);
    }

    const auto target = direction_to(
        current.ball_x_raw(),
        current.ball_y_raw(),
        target_x,
        target_y);

    static constexpr std::array<int, 51> offsets{{
        0,
        -1, 1, -2, 2, -3, 3, -4, 4, -5, 5, -6, 6, -7, 7, -8, 8,
        -12, 12, -16, 16, -24, 24, -32, 32, -48, 48, -64, 64,
        -96, 96, -128, 128, -192, 192, -256, 256, -384, 384,
        -512, 512, -768, 768, -1024, 1024, -1280, 1280,
        -1536, 1536, -1792, 1792
    }};
    static constexpr std::array<int, 23> powers{{
        105, 95, 85, 75, 65, 55, 45, 35, 25,
        20, 18, 16, 14, 12, 10, 8, 6, 5, 4, 3, 2, 1, 0
    }};

    std::vector<std::uint16_t> clubs;
    if (surface.landing_code == 1
        || surface.landing_code == 8
        || surface.landing_code == 9
        || surface.landing_code == 10) {
        clubs.push_back(12);
    } else {
        for (std::uint16_t club = 0; club <= 12; ++club) {
            clubs.push_back(club);
        }
    }

    ShotChoice best{};
    for (const auto club : clubs) {
        for (const auto power : powers) {
            for (const auto offset : offsets) {
                auto trial = current;
                sensigolf::ClassicShotRequest request{};
                request.club_index = club;
                request.power_tick = power;
                request.accuracy_tick = 63;
                request.aim_raw = static_cast<std::uint16_t>(
                    static_cast<int>(target) + offset) & 0x0FFFu;

                try {
                    trial.begin_shot(request);
                    run_until_terminal(trial);
                } catch (const std::exception&) {
                    continue;
                }

                const auto phase = trial.phase();
                if (phase == sensigolf::HoleSessionPhase::UnsupportedTerrain
                    || phase == sensigolf::HoleSessionPhase::HazardStopped) {
                    continue;
                }

                const auto distance = trial.distance_to_hole();
                if (phase == sensigolf::HoleSessionPhase::CupTerminal
                    && distance == 0) {
                    return ShotChoice{request, 0, phase, true};
                }

                // Prefer clean resting shots. Special-green terminal positions
                // remain legal candidates, but lose ties to normal rests.
                const auto penalty =
                    phase == sensigolf::HoleSessionPhase::SpecialGreenStopped
                    ? 8u : 0u;
                const auto score = distance > std::numeric_limits<std::uint32_t>::max() - penalty
                    ? distance
                    : distance + penalty;

                const auto best_score =
                    best.resulting_distance
                    + (best.phase == sensigolf::HoleSessionPhase::SpecialGreenStopped ? 8u : 0u);
                if (!best.valid || score < best_score) {
                    best = ShotChoice{request, distance, phase, true};
                }
            }
        }
    }
    return best;
}

void apply_shot(
    sensigolf::ClassicHoleSession& hole,
    const sensigolf::ClassicShotRequest& request) {
    hole.begin_shot(request);
    run_until_terminal(hole);
    finish_replay_gate(hole);
    if (hole.phase() == sensigolf::HoleSessionPhase::CupTerminal
        && hole.distance_to_hole() == 0) {
        hole.step();
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 11) {
        std::cerr
            << "usage: sensigolf_real_hole_solver_probe "
            << "<resource-id> <par> <next-resource-id> <next-par> "
            << "<mapm> <maps> <spt> <mapi-desc> <mapi-select> <player-slot>\n";
        return 2;
    }

    try {
        const auto resource_id = static_cast<std::uint8_t>(
            std::stoul(argv[1], nullptr, 0));
        const auto par = static_cast<std::uint8_t>(
            std::stoul(argv[2], nullptr, 0));
        const auto next_resource_id = static_cast<std::uint8_t>(
            std::stoul(argv[3], nullptr, 0));
        const auto next_par = static_cast<std::uint8_t>(
            std::stoul(argv[4], nullptr, 0));
        const auto player_slot = static_cast<std::size_t>(
            std::stoul(argv[10], nullptr, 0));

        const auto mapm = read_file(argv[5]);
        const auto maps = read_file(argv[6]);
        const auto spt = read_file(argv[7]);
        const auto desc = read_file(argv[8]);
        const auto sel = read_file(argv[9]);

        auto make_course = [&]() {
            return sensigolf::ClassicCourseResources(
                mapm, spt, desc, sel, maps);
        };

        auto search_course = make_course();
        const auto green_region = search_course.green_region();
        if (green_region) {
            const auto cup = search_course.hole_position();
            const auto gx = static_cast<int>(cup.x) - green_region->origin_x;
            const auto gy = static_cast<int>(cup.y) - green_region->origin_y;
            std::cerr << "diagnostic green origin=("
                      << green_region->origin_x << ","
                      << green_region->origin_y << ") cup=("
                      << cup.x << "," << cup.y
                      << ") local=(" << gx << "," << gy << ")"
                      << " green_scaled_target=(" << (gx * 2)
                      << "," << (gy * 2) << ")\n";
            int code8_count = 0;
            int code8_best_error = 1000000;
            int code8_best_x = -1;
            int code8_best_y = -1;
            for (int gy2 = 0; gy2 < 208; ++gy2) {
                for (int gx2 = 0; gx2 < 256; ++gx2) {
                    const auto s = search_course.resolve_integer_position(
                        static_cast<std::int16_t>(gx2),
                        static_cast<std::int16_t>(gy2), true);
                    if (s.landing_code != 8) continue;
                    ++code8_count;
                    const auto error = std::abs(gx2 - gx * 2)
                        + std::abs(gy2 - gy * 2);
                    if (error < code8_best_error) {
                        code8_best_error = error;
                        code8_best_x = gx2;
                        code8_best_y = gy2;
                    }
                }
            }
            std::cerr << "diagnostic code8 cells=" << code8_count
                      << " nearest_code8=(" << code8_best_x
                      << "," << code8_best_y << ") delta="
                      << code8_best_error << "\n";
        }
        sensigolf::ClassicHoleSession search(
            search_course,
            player_slot,
            sensigolf::ClassicHoleMetadata{0, par, resource_id});

        std::vector<sensigolf::ClassicShotRequest> shots;
        for (int stroke = 0; stroke < 12; ++stroke) {
            if (search.phase() == sensigolf::HoleSessionPhase::HoleScored) {
                break;
            }
            if (search.phase() != sensigolf::HoleSessionPhase::ReadyForShot) {
                throw std::runtime_error("solver reached non-playable phase");
            }

            const auto before = search.distance_to_hole();
            const auto choice = choose_shot(
                search_course,
                search);
            if (!choice.valid) {
                throw std::runtime_error("solver found no legal improving shot");
            }

            const auto current_surface = search.current_surface();
            std::cerr
                << "solver stroke " << (stroke + 1)
                << " before=" << before
                << " green=" << search.green_mode()
                << " surface_code=" << current_surface.landing_code
                << " descriptor=" << current_surface.descriptor_index
                << " club=" << choice.request.club_index
                << " power=" << choice.request.power_tick
                << " aim=" << choice.request.aim_raw
                << " predicted=" << choice.resulting_distance
                << " predicted_phase=" << static_cast<unsigned>(choice.phase)
                << "\n";

            const auto old_x = search.ball_x_raw();
            const auto old_y = search.ball_y_raw();
            apply_shot(search, choice.request);
            shots.push_back(choice.request);

            if (search.phase() != sensigolf::HoleSessionPhase::HoleScored) {
                const auto after = search.distance_to_hole();
                std::cerr
                    << "solver result " << (stroke + 1)
                    << " after=" << after
                    << " x_raw=" << search.ball_x_raw()
                    << " y_raw=" << search.ball_y_raw()
                    << " phase=" << static_cast<unsigned>(search.phase())
                    << " green=" << search.green_mode()
                    << " surface=" << search.current_surface().landing_code
                    << "\n";
                if (search.ball_x_raw() == old_x
                    && search.ball_y_raw() == old_y) {
                    throw std::runtime_error(
                        "solver selected a shot with no positional progress");
                }
            }
        }

        if (search.phase() != sensigolf::HoleSessionPhase::HoleScored) {
            throw std::runtime_error("real resource hole was not completed");
        }

        std::array<std::uint8_t, sensigolf::kClassicRoundHoleCount> order{};
        order.fill(next_resource_id);
        order[0] = resource_id;
        order[1] = next_resource_id;
        std::array<std::uint8_t, sensigolf::kClassicParTableSize> pars{};
        pars[resource_id] = par;
        pars[next_resource_id] = next_par;

        sensigolf::ClassicGameSession game(
            sensigolf::ClassicHolePlan(order, pars),
            player_slot);
        game.load_current_hole(resource_id, make_course());

        for (const auto& shot : shots) {
            apply_shot(game.active_hole(), shot);
            if (game.active_hole().phase()
                == sensigolf::HoleSessionPhase::HoleScored) {
                break;
            }
        }

        if (game.active_hole().phase()
            != sensigolf::HoleSessionPhase::HoleScored) {
            throw std::runtime_error("game-session replay did not score real hole");
        }
        game.commit_scored_hole();

        const auto next = game.resource_request();
        if (!next || next->resource_id != next_resource_id
            || next->round_index != 1u) {
            throw std::runtime_error(
                "game-session replay did not activate expected next resource");
        }

        std::cout << "{"
                  << "\"resource_id\":" << static_cast<unsigned>(resource_id)
                  << ",\"par\":" << static_cast<unsigned>(par)
                  << ",\"strokes\":" << shots.size()
                  << ",\"next_resource_id\":" << static_cast<unsigned>(next->resource_id)
                  << ",\"next_round_index\":" << next->round_index
                  << ",\"shots\":[";
        for (std::size_t i = 0; i < shots.size(); ++i) {
            if (i != 0) std::cout << ",";
            std::cout << "{"
                      << "\"club\":" << shots[i].club_index
                      << ",\"power\":" << shots[i].power_tick
                      << ",\"accuracy\":" << shots[i].accuracy_tick
                      << ",\"aim\":" << shots[i].aim_raw
                      << "}";
        }
        std::cout << "]}\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
