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

std::vector<std::uint8_t> read_file(const char* path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error(std::string("cannot open ") + path);
    }
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>());
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 8) {
        std::cerr
            << "usage: sensigolf_classic_game_session_probe "
            << "<resource-id> <par> <mapm> <spt> <mapi-desc> <mapi-select> "
            << "<player-slot>\n";
        return 2;
    }

    try {
        const auto resource_raw = std::stoul(argv[1], nullptr, 0);
        const auto par_raw = std::stoul(argv[2], nullptr, 0);
        const auto player_slot = std::stoul(argv[7], nullptr, 0);
        if (resource_raw == 0u || resource_raw >= 100u
            || par_raw == 0u || par_raw > 255u
            || player_slot >= 4u) {
            throw std::out_of_range("probe argument outside recovered range");
        }

        const auto resource_id = static_cast<std::uint8_t>(resource_raw);
        const auto par = static_cast<std::uint8_t>(par_raw);

        std::array<std::uint8_t, sensigolf::kClassicRoundHoleCount> order{};
        order.fill(resource_id);
        std::array<std::uint8_t, sensigolf::kClassicParTableSize> pars{};
        pars[resource_id] = par;

        sensigolf::ClassicGameSession game(
            sensigolf::ClassicHolePlan(order, pars),
            player_slot);

        const auto request = game.resource_request();
        if (!request) {
            throw std::runtime_error("missing first-hole resource request");
        }

        sensigolf::ClassicCourseResources course(
            read_file(argv[3]),
            read_file(argv[4]),
            read_file(argv[5]),
            read_file(argv[6]));
        const auto tee = course.player_start(player_slot);
        const auto cup = course.hole_position();
        std::array<sensigolf::RawSptRecord, sensigolf::ClassicCourseResources::kSptRecordCount> spt_rows{};
        for (std::size_t i = 0; i < spt_rows.size(); ++i) {
            spt_rows[i] = course.spt_record(i);
        }

        game.load_current_hole(resource_id, std::move(course));
        const auto metadata = game.active_hole().hole_metadata();
        if (!metadata) {
            throw std::runtime_error("active real hole missing metadata");
        }

        std::cout
            << "{"
            << "\"resource_id\":" << static_cast<unsigned>(request->resource_id)
            << ",\"par\":" << static_cast<unsigned>(request->par)
            << ",\"round_index\":" << request->round_index
            << ",\"mapm_map\":\"" << request->resources.mapm_map << "\""
            << ",\"maps_map\":\"" << request->resources.maps_map << "\""
            << ",\"mapm_spt\":\"" << request->resources.mapm_spt << "\""
            << ",\"tee_x\":" << tee.x
            << ",\"tee_y\":" << tee.y
            << ",\"cup_x\":" << cup.x
            << ",\"cup_y\":" << cup.y
            << ",\"ball_x_raw\":" << game.active_hole().ball_x_raw()
            << ",\"ball_y_raw\":" << game.active_hole().ball_y_raw()
            << ",\"metadata_resource_id\":" << metadata->resource_id
            << ",\"metadata_par\":" << metadata->par
            << ",\"spt_records\":[";
        for (std::size_t i = 0; i < spt_rows.size(); ++i) {
            if (i != 0) std::cout << ",";
            std::cout << "[";
            for (std::size_t j = 0; j < spt_rows[i].words.size(); ++j) {
                if (j != 0) std::cout << ",";
                std::cout << spt_rows[i].words[j];
            }
            std::cout << "]";
        }
        std::cout
            << "]"
            << ",\"ready\":"
            << (game.active_hole().phase() == sensigolf::HoleSessionPhase::ReadyForShot
                ? "true" : "false")
            << "}\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
