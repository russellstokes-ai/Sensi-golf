#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "sensigolf/classic_hole_session.hpp"

using namespace sensigolf;

namespace {

struct ShotChoice {
    std::uint16_t aim = 0;
    std::int32_t power = 0;
    std::uint16_t club = 0;
};

struct Candidate {
    ShotChoice shot{};
    std::unique_ptr<ClassicHoleSession> session{};
    std::uint32_t distance = std::numeric_limits<std::uint32_t>::max();
    std::uint16_t landing_code = 0;
    bool scored = false;
};

std::uint32_t route_cost(const Candidate& candidate) {
    if (candidate.scored) return 0;

    // Close to the cup, a normal green lie is strategically more useful than
    // a numerically closer fairway/rough lie because club 12 is the recovered
    // finishing path. This prevents the search from parking beside the cup on
    // a non-putter surface.
    const auto lie_penalty =
        candidate.distance <= 100u && candidate.landing_code != 1u
        ? 500u : 0u;
    return candidate.distance + lie_penalty;
}

std::vector<std::uint8_t> read_file(const char* path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error(std::string("cannot open ") + path);
    }
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>());
}

std::uint16_t wrap_aim(int value) {
    value %= 4096;
    if (value < 0) value += 4096;
    return static_cast<std::uint16_t>(value);
}

std::unique_ptr<Candidate> try_shot(
    const ClassicHoleSession& base,
    const ShotChoice& choice) {
    auto trial = std::make_unique<ClassicHoleSession>(base);

    ClassicShotRequest request{};
    request.aim_raw = choice.aim;
    request.power_tick = choice.power;
    request.accuracy_tick = 63;
    request.club_index = choice.club;

    try {
        trial->begin_shot(request);

        int ticks = 0;
        while (trial->phase() == HoleSessionPhase::ShotActive
               && ticks++ < 4096) {
            trial->step();
        }
        if (ticks >= 4096) {
            return nullptr;
        }

        if (trial->phase() == HoleSessionPhase::CupTerminal) {
            trial->step();
        }

        if (trial->phase() != HoleSessionPhase::ReadyForShot
            && trial->phase() != HoleSessionPhase::HoleScored) {
            return nullptr;
        }

        auto out = std::make_unique<Candidate>();
        out->shot = choice;
        out->distance = trial->distance_to_hole();
        out->scored = trial->phase() == HoleSessionPhase::HoleScored;
        out->landing_code = out->scored
            ? 8u
            : trial->current_surface().landing_code;
        out->session = std::move(trial);
        return out;
    } catch (...) {
        // Search-only candidates may leave the finite MAPM grid or hit a
        // not-yet-integrated terrain rule. They are invalid routes, not a
        // fatal error for the deterministic search.
        return nullptr;
    }
}

void retain_best(
    std::vector<std::unique_ptr<Candidate>>& best,
    std::unique_ptr<Candidate> candidate,
    std::size_t limit) {
    if (!candidate) return;
    best.push_back(std::move(candidate));
    std::sort(
        best.begin(),
        best.end(),
        [](const auto& a, const auto& b) {
            if (a->scored != b->scored) return a->scored > b->scored;
            const auto ac = route_cost(*a);
            const auto bc = route_cost(*b);
            if (ac != bc) return ac < bc;
            return a->distance < b->distance;
        });
    if (best.size() > limit) best.resize(limit);
}

std::vector<ShotChoice> coarse_choices(
    const ClassicHoleSession& session) {
    std::vector<ShotChoice> out;
    const auto surface = session.current_surface();

    if (surface.landing_code == 1
        || surface.landing_code == 8
        || surface.landing_code == 9
        || surface.landing_code == 10) {
        for (int aim = 0; aim < 4096; aim += 64) {
            for (int power = 5; power <= 105; power += 5) {
                out.push_back(ShotChoice{
                    static_cast<std::uint16_t>(aim),
                    power,
                    12u});
            }
        }
        return out;
    }

    constexpr std::array<int, 6> powers{
        45, 60, 75, 90, 100, 105};
    for (int aim = 0; aim < 4096; aim += 128) {
        for (std::uint16_t club = 0; club < 12; ++club) {
            for (const auto power : powers) {
                out.push_back(ShotChoice{
                    static_cast<std::uint16_t>(aim),
                    power,
                    club});
            }
        }
    }
    return out;
}

std::vector<ShotChoice> refine_choices(
    const std::vector<std::unique_ptr<Candidate>>& coarse) {
    std::vector<ShotChoice> out;
    for (const auto& base : coarse) {
        const auto club_min = base->shot.club == 0
            ? 0 : static_cast<int>(base->shot.club) - 1;
        const auto club_max = base->shot.club == 12
            ? 12 : std::min(12, static_cast<int>(base->shot.club) + 1);

        for (int club = club_min; club <= club_max; ++club) {
            // Do not cross between putter and normal clubs during refinement.
            if ((base->shot.club == 12) != (club == 12)) continue;
            for (int da = -128; da <= 128; da += 16) {
                for (int dp = -12; dp <= 12; dp += 3) {
                    const auto power = base->shot.power + dp;
                    if (power < 1 || power > 105) continue;
                    out.push_back(ShotChoice{
                        wrap_aim(static_cast<int>(base->shot.aim) + da),
                        power,
                        static_cast<std::uint16_t>(club)});
                }
            }
        }
    }
    return out;
}

std::unique_ptr<Candidate> best_next_shot(
    const ClassicHoleSession& session) {
    std::vector<std::unique_ptr<Candidate>> coarse;
    for (const auto& shot : coarse_choices(session)) {
        retain_best(coarse, try_shot(session, shot), 12);
        if (!coarse.empty() && coarse.front()->scored) {
            return std::move(coarse.front());
        }
    }

    if (coarse.empty()) return nullptr;

    std::vector<std::unique_ptr<Candidate>> refined;
    for (const auto& shot : refine_choices(coarse)) {
        retain_best(refined, try_shot(session, shot), 12);
        if (!refined.empty() && refined.front()->scored) {
            return std::move(refined.front());
        }
    }

    if (!refined.empty()
        && refined.front()->distance <= coarse.front()->distance) {
        return std::move(refined.front());
    }
    return std::move(coarse.front());
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 5) {
        std::cerr
            << "usage: sensigolf_real_hole_search "
            << "<mapm> <spt> <mapi-desc> <mapi-select>\n";
        return 2;
    }

    try {
        ClassicCourseResources course(
            read_file(argv[1]),
            read_file(argv[2]),
            read_file(argv[3]),
            read_file(argv[4]));

        auto session = std::make_unique<ClassicHoleSession>(
            course,
            0u,
            ClassicHoleMetadata{0u, 4u, 42u});

        const auto tee = course.player_start(0);
        const auto cup = course.hole_position();
        std::cout
            << "START tee=(" << tee.x << "," << tee.y << ")"
            << " cup=(" << cup.x << "," << cup.y << ")"
            << " distance=" << session->distance_to_hole()
            << "\n";

        constexpr int kMaxShots = 12;
        for (int stroke = 1; stroke <= kMaxShots; ++stroke) {
            const auto before = session->distance_to_hole();
            auto best = best_next_shot(*session);
            if (!best) {
                std::cout << "NO_CANDIDATE stroke=" << stroke << "\n";
                break;
            }

            const auto x = best->session->ball_x_raw() >> 16;
            const auto y = best->session->ball_y_raw() >> 16;
            std::cout
                << "SHOT " << stroke
                << " club=" << best->shot.club
                << " power=" << best->shot.power
                << " aim=" << best->shot.aim
                << " from_distance=" << before
                << " to_distance=" << best->distance
                << " pos=(" << x << "," << y << ")"
                << " phase=" << static_cast<int>(best->session->phase())
                << " surface=" << best->landing_code
                << " route_cost=" << route_cost(*best)
                << "\n";

            session = std::move(best->session);

            if (session->phase() == HoleSessionPhase::HoleScored) {
                std::cout << "FOUND strokes=" << stroke << "\n";
                return 0;
            }

            // Allow a modest distance increase when it purchases a proper
            // green lie; otherwise stop obvious non-progress loops.
            const auto current_surface = session->current_surface();
            if (session->distance_to_hole() >= before
                && current_surface.landing_code != 1u) {
                std::cout
                    << "STALLED distance=" << session->distance_to_hole()
                    << "\n";
                break;
            }
        }

        std::cout
            << "NOT_FOUND final_distance=" << session->distance_to_hole()
            << " strokes=" << session->strokes()
            << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
