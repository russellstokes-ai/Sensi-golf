#include "sensigolf/classic_shot_model.hpp"

#include <cstdint>
#include <algorithm>
#include <limits>
#include <stdexcept>

#include "sensigolf/recovered_ground.hpp"
#include "sensigolf/recovered_interactions.hpp"
#include "sensigolf/recovered_putter_terminal.hpp"

namespace sensigolf {
namespace {

std::int64_t sar_floor(std::int64_t value, unsigned bits) {
    if (value >= 0) return value >> bits;
    const auto divisor = std::int64_t{1} << bits;
    return -(((-value) + divisor - 1) / divisor);
}

std::int32_t planar_component(
    std::int32_t force,
    std::int16_t trig) {
    const auto wide =
        static_cast<std::int64_t>(force) * static_cast<std::int64_t>(trig);
    const auto shifted = sar_floor(wide, 14);
    if (shifted < std::numeric_limits<std::int32_t>::min()
        || shifted > std::numeric_limits<std::int32_t>::max()) {
        throw std::overflow_error("classic shot planar component overflow");
    }
    return static_cast<std::int32_t>(shifted);
}

} // namespace

void ClassicShotModel::reset() {
    wind_={};wind_accumulator_={};
    flight_ = {};
    public_ = {};
    surface_ = {};
    origin_x_ = 0;
    origin_y_ = 0;
    active_club_ = 0;
    outcome_ = ClassicShotOutcome::None;
    active_ = false;
    soft_ground_enabled_=false;soft_ground_rolling_=false;
    interaction_prng_ = nullptr;
    interaction_green_mode_ = false;
    flag_x_ = 0;
    flag_y_ = 0;
    flag_object_verified_ = false;
    near_cup_distance_zero_ = false;
    player_mode_5a_ = 4;
    lip_deflection_last_tick_ = false;
    presentation_contact_ = {};
    authored_trees_=nullptr;last_tree_id_=0;
}

void ClassicShotModel::set_original_interaction_context(
    recovered::OriginalPrng16* prng, bool green_mode,
    std::int16_t flag_x, std::int16_t flag_y,
    bool flag_object_verified,bool near_cup_distance_zero,
    std::uint16_t player_mode_5a) noexcept {
    interaction_prng_ = prng;
    interaction_green_mode_ = green_mode;
    flag_x_ = flag_x;
    flag_y_ = flag_y;
    flag_object_verified_ = flag_object_verified;
    near_cup_distance_zero_=near_cup_distance_zero;
    player_mode_5a_=player_mode_5a;
}

void ClassicShotModel::set_surface_context(
    const ClassicSurfaceContext& context) {
    if (active_) {
        throw std::logic_error("cannot change surface context during an active shot");
    }
    surface_ = context;
}

void ClassicShotModel::update_surface_context_for_tick(
    const ClassicSurfaceContext& context) {
    if (!active_) {
        throw std::logic_error(
            "tick surface context requires an active shot");
    }
    surface_ = context;
    public_.surface_index = context.landing_code;
}

const ClassicSurfaceContext& ClassicShotModel::surface_context() const noexcept {
    return surface_;
}

void ClassicShotModel::set_ball_origin(
    std::int32_t x_raw,
    std::int32_t y_raw) noexcept {
    if (active_) {
        return;
    }
    origin_x_ = x_raw;
    origin_y_ = y_raw;
}

void ClassicShotModel::begin_shot(const ShotInput& input) {
    soft_ground_rolling_=false;
    presentation_contact_ = {};
    if (active_) {
        throw std::logic_error("shot already active");
    }
    if (input.aim_raw < 0 || input.aim_raw > recovered::kDirectionMask) {
        throw std::out_of_range("aim_raw must be original direction 0..4095");
    }

    recovered::LaunchInput launch{};
    launch.club_index = input.club_index;
    launch.lie_index = input.lie_index;
    launch.captured_power = input.power_tick;
    launch.accuracy_tick = input.accuracy_tick;
    launch.player_direction = static_cast<std::uint16_t>(input.aim_raw);
    launch.start_x = origin_x_;
    launch.start_y = origin_y_;

    wind_accumulator_={};
    flight_ = recovered::launch_normal_shot(launch);
    active_club_ = input.club_index;
    outcome_ = ClassicShotOutcome::None;
    active_ = true;

    public_ = {};
    public_.phase = active_club_ == 12
        ? ShotPhase::GroundRoll
        : ShotPhase::Airborne;
    sync_public_state();
}

void ClassicShotModel::step() {
    presentation_contact_ = {};
    if (!active_) {
        return;
    }

    ++public_.tick;
    lip_deflection_last_tick_=false;
    presentation_contact_.green_mode=interaction_green_mode_;
    presentation_contact_.landing_code=surface_.landing_code;
    presentation_contact_.x_raw=flight_.x;
    presentation_contact_.y_raw=flight_.y;
    const auto incoming_height=flight_.height;
    const auto previous_flight=flight_;

    // Source-disassembled v1.014 0x40A374..0x40A575: code 9 and flag
    // interactions precede this tick's normal gravity/putter response.
    if(active_club_ != 12) {
        const auto code9 = recovered::original_code9_contact(
            surface_.landing_code, active_club_,
            flight_.vertical_force, flight_.height);
        if(code9 == recovered::Code9Contact::SpecialStop) {
            active_ = false;
            outcome_ = ClassicShotOutcome::SpecialGreenStop;
            public_.phase = ShotPhase::Complete;
            sync_public_state();
            return;
        }
        if(code9 == recovered::Code9Contact::Deflect) {
            if(!interaction_prng_)
                throw std::logic_error("code9 impact needs original captured PRNG");
            recovered::apply_code9_low_height_deflection(
                flight_, *interaction_prng_);
        }
    }
    // Standalone physics fixtures may have no original PRNG. Their
    // non-interaction behaviour remains testable; the real game session
    // always supplies the captured original PRNG for event activation.
    // Original v1.014 0x40A456..0x40A4A6 near-cup lip contact occurs
    // after code9 but before the flagpole check and normal shot advance.
    // It must not activate from an inferred source player mode or a
    // stale saved ball position. It consumes precisely one original PRNG
    // call only on the source-proven high-speed lip branch.
    if(near_cup_distance_zero_ && interaction_prng_ &&
       recovered::original_near_cup_lip_contact(
           true,player_mode_5a_,flight_)) {
        lip_deflection_last_tick_=recovered::apply_near_hole_lip_deflection(
            flight_,*interaction_prng_);
        presentation_contact_.lip=lip_deflection_last_tick_;
    }
    if(flag_object_verified_ && interaction_prng_ &&
       recovered::original_flag_coordinate_hit(
           flight_, flag_x_, flag_y_, interaction_green_mode_)) {
        recovered::apply_flag_coordinate_deflection(
            flight_, *interaction_prng_);
        presentation_contact_.flag=true;
    }

    if(presentation_contact_.flag || presentation_contact_.lip)wind_accumulator_={};
    if(active_club_!=12 && !soft_ground_rolling_)apply_authored_wind(flight_,wind_,wind_accumulator_,interaction_green_mode_);
    const auto incoming_horizontal=flight_.horizontal_force;
    recovered::GroundStepResult result{};
    if (active_club_ == 12) {
        recovered::PutterTerminalState terminal{};
        if (recovered::apply_putter_terminal(
                surface_.landing_code, terminal)) {
            active_ = false;
            public_.phase = ShotPhase::Complete;
            if (terminal.kind
                == recovered::PutterTerminalKind::Holed) {
                public_.holed = true;
                outcome_ = ClassicShotOutcome::CupTerminal;
            } else {
                outcome_ = ClassicShotOutcome::SpecialGreenStop;
            }
            sync_public_state();
            return;
        }

        if (surface_.landing_code >= 1
                   && surface_.landing_code <= 7) {
            // Original scene flag controls drag/slope on every ordinary lie.
            // Descriptor-only routing incorrectly slowed MAPM green putts and
            // applied full drag to MAPS fringe. Source12-trace matrix proves it.
            result = interaction_green_mode_
                ? recovered::step_green_putt(flight_,surface_.slope_direction,surface_.slope_magnitude)
                : recovered::step_non_green_putt(flight_);
        } else {
            throw std::logic_error(
                "club-12 surface is not yet product-integrated");
        }
    } else if(soft_ground_enabled_ && soft_ground_rolling_ && surface_.landing_code>=1 && surface_.landing_code<=7) {
        result=interaction_green_mode_
            ? recovered::step_green_putt(flight_,surface_.slope_direction,surface_.slope_magnitude)
            : recovered::step_non_green_putt(flight_);
        flight_.vertical_force=0;
    } else {
        result = recovered::step_controlled_surface(
            flight_,
            surface_.landing_code);
    }

    // Opt-in Pixtee authoring response, not recovered source parity: rough and
    // sand absorb the first impact; subsequent movement uses the native ground path.
    if(soft_ground_enabled_ && result.contacted_ground && surface_.landing_code>=5 && surface_.landing_code<=7) {
        if(!soft_ground_rolling_)flight_.horizontal_force=std::max(0,incoming_horizontal/(surface_.landing_code==7?4:2));
        soft_ground_rolling_=true;flight_.height=0;flight_.vertical_force=0;
        result.resting=flight_.horizontal_force==0;
    }
    // A source-ground branch reached from airborne height is an impact;
    // subsequent rolling contacts are not repeated particle/audio impacts.
    if(result.contacted_ground)wind_accumulator_={};
    presentation_contact_.impact=result.contacted_ground && incoming_height>0;
    presentation_contact_.hazard=result.hazard_stop;
    if(result.contacted_ground) {
        presentation_contact_.x_raw=result.contact_x;
        presentation_contact_.y_raw=result.contact_y;
    }
    if (result.contacted_ground && !result.resting) {
        public_.phase = ShotPhase::GroundRoll;
    }
    if (result.hazard_stop) {
        public_.hazard = true;
        outcome_ = ClassicShotOutcome::Hazard;
    }
    if (result.holed) {
        public_.holed = true;
        outcome_ = ClassicShotOutcome::CupTerminal;
    }
    if (result.resting) {
        active_ = false;
        public_.phase = ShotPhase::Complete;
        if (outcome_ == ClassicShotOutcome::None) {
            outcome_ = ClassicShotOutcome::Rest;
        }
    }

    if(active_ && authored_trees_ && !authored_trees_->empty()) {
        auto point=[&](const recovered::FlightState& state) {
            return AuthoredTreePoint{tree_green_?state.x/2+tree_origin_x_:state.x,
                tree_green_?state.y/2+tree_origin_y_:state.y,state.height};
        };
        const auto contact=authored_tree_contact(*authored_trees_,point(previous_flight),point(flight_));
        if(contact.tree_id && contact.tree_id!=last_tree_id_) {
            if(!interaction_prng_)throw std::logic_error("authored tree impact requires native PRNG");
            if(contact.tree_id>=100 && contact.tree_id<104) {
                // Authored Lakewood sponsor boards: solid face, deterministic rebound.
                flight_.direction=static_cast<std::uint16_t>((previous_flight.direction+2048u)&4095u);
                flight_.horizontal_force=std::max(0,flight_.horizontal_force/2);
                const auto board=std::find_if(authored_trees_->begin(),authored_trees_->end(),[&](const auto& object){return object.id==contact.tree_id;});
                if(board!=authored_trees_->end() && previous_flight.height>=board->trunk_height && flight_.vertical_force<0) {
                    // A descending ball can strike the top after its horizontal
                    // force has already reached zero. Reflect its actual vertical
                    // motion using the recovered rebound primitive.
                    const auto rebound=recovered::original_terminal_rebound(flight_.vertical_force,flight_.horizontal_force);
                    flight_.vertical_force=rebound.vertical_force;
                    flight_.horizontal_force=rebound.horizontal_force;
                    flight_.height=board->trunk_height;
                }
            }else recovered::apply_code9_low_height_deflection(flight_,*interaction_prng_);
            if(contact.kind==AuthoredTreeContactKind::Canopy && !(contact.tree_id>=100 && contact.tree_id<104)) {
                const auto rebound=recovered::original_terminal_rebound(flight_.vertical_force,flight_.horizontal_force);
                flight_.vertical_force=rebound.vertical_force;
                flight_.horizontal_force=rebound.horizontal_force;
            }
            presentation_contact_.tree_id=contact.tree_id;
            presentation_contact_.canopy=contact.kind==AuthoredTreeContactKind::Canopy;
            presentation_contact_.x_raw=previous_flight.x;presentation_contact_.y_raw=previous_flight.y;
        }
        // Dedupe only PRNG/event emission. Every intersecting tick still
        // prevents penetration, even if a grazing deflection hits again.
        if(contact.tree_id) {
            flight_.x=previous_flight.x;flight_.y=previous_flight.y;
        }
        last_tree_id_=contact.tree_id;
    }
    if(active_ && bird_hit_ && !*bird_hit_) {
        const auto point=[&](const recovered::FlightState& state) {
            return AuthoredTreePoint{tree_green_?state.x/2+tree_origin_x_:state.x,
                tree_green_?state.y/2+tree_origin_y_:state.y,state.height};
        };
        unsigned hit_member=0;
        if(authored_bird_contact(bird_before_,bird_after_,point(previous_flight),point(flight_),&hit_member)) {
            if(!interaction_prng_)throw std::logic_error("bird impact requires native PRNG");
            recovered::apply_code9_low_height_deflection(flight_,*interaction_prng_);
            const auto rebound=recovered::original_terminal_rebound(flight_.vertical_force,flight_.horizontal_force);
            flight_.vertical_force=rebound.vertical_force;flight_.horizontal_force=rebound.horizontal_force;
            *bird_hit_=true;
            presentation_contact_.bird_id=bird_after_.id;
            presentation_contact_.bird_member=hit_member;
            presentation_contact_.x_raw=previous_flight.x;presentation_contact_.y_raw=previous_flight.y;
        }
    }
    sync_public_state();
}

void ClassicShotModel::sync_public_state() {
    if(presentation_contact_.tree_id || presentation_contact_.bird_id)wind_accumulator_={};
    public_.x_raw = flight_.x;
    public_.y_raw = flight_.y;
    public_.z_raw = flight_.height;

    public_.vx_raw = planar_component(
        flight_.horizontal_force,
        recovered::trig_q14(flight_.direction));
    public_.vy_raw = planar_component(
        flight_.horizontal_force,
        recovered::trig_q14(
            static_cast<std::uint16_t>(flight_.direction + 1024)));
    public_.vz_raw = flight_.vertical_force;
    public_.surface_index = surface_.landing_code;
}

void ClassicShotModel::stop_for_world_boundary() {
    if(!active_) {
        throw std::logic_error("world boundary stop requires an active shot");
    }
    active_=false;
    outcome_=ClassicShotOutcome::Hazard;
    public_.hazard=true;
    public_.phase=ShotPhase::Complete;
    // Original negative-AX branch does not use the ordinary landing
    // collision zeroing; flight forces are left untouched until original
    // position-restoration handling. Do not manufacture a tile-35 impact.
    sync_public_state();
}

void ClassicShotModel::relocate_inactive_ball(
    std::int32_t x_raw,
    std::int32_t y_raw,
    std::uint16_t surface_index) {
    if (active_) {
        throw std::logic_error("cannot relocate an active shot");
    }

    flight_.x = x_raw;
    flight_.y = y_raw;
    flight_.height = 0;
    flight_.vertical_force = 0;
    flight_.horizontal_force = 0;
    origin_x_ = x_raw;
    origin_y_ = y_raw;

    public_.hazard = false;
    public_.holed = false;
    public_.surface_index = surface_index;
    public_.phase = ShotPhase::Complete;
    sync_public_state();
}

void ClassicShotModel::enter_green_coordinates(
    std::int32_t origin_x_raw,
    std::int32_t origin_y_raw) {
    // Original 0x40AB3C..0x40AB5C uses 32-bit SUB then SHL 1.
    flight_.x = static_cast<std::int32_t>(
        (static_cast<std::uint32_t>(flight_.x)
         - static_cast<std::uint32_t>(origin_x_raw)) << 1);
    flight_.y = static_cast<std::int32_t>(
        (static_cast<std::uint32_t>(flight_.y)
         - static_cast<std::uint32_t>(origin_y_raw)) << 1);
    sync_public_state();
}

void ClassicShotModel::leave_green_coordinates(
    std::int32_t origin_x_raw,
    std::int32_t origin_y_raw) {
    // Original 0x40ABB8..0x40ABD8 uses arithmetic SAR 1 then ADD origin.
    const auto half_x = sar_floor(flight_.x, 1);
    const auto half_y = sar_floor(flight_.y, 1);
    flight_.x = static_cast<std::int32_t>(
        static_cast<std::uint32_t>(half_x)
        + static_cast<std::uint32_t>(origin_x_raw));
    flight_.y = static_cast<std::int32_t>(
        static_cast<std::uint32_t>(half_y)
        + static_cast<std::uint32_t>(origin_y_raw));
    sync_public_state();
}

const BallState& ClassicShotModel::state() const {
    return public_;
}

bool ClassicShotModel::shot_active() const {
    return active_;
}

std::uint32_t ClassicShotModel::tick_rate_hz() const {
    return kNominalTickRateHz;
}

} // namespace sensigolf
