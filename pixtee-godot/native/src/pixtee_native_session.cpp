#include "pixtee_round_runtime.hpp"
#include <sstream>
#include <stdexcept>
#include <utility>
namespace pixtee {
void PixteeRoundRuntime::record_command(char op,std::vector<std::int64_t> args) {
    if(!recording_)return;
    if(!commands_.empty() && commands_.back().op==op && commands_.back().args==args && (op=='S'||op=='P')) {
        ++commands_.back().repeat;return;
    }
    commands_.push_back({op,std::move(args),1});
}
std::string PixteeRoundRuntime::native_fingerprint() const {
    std::ostringstream out;
    const auto& r=round_state();
    out<<r.current_hole_index<<' '<<r.holes_completed<<' '<<r.total_strokes<<' '<<r.cumulative_par<<' '<<r.relative_to_par<<' '<<r.round_complete;
    out<<' '<<controls_.aim_raw()<<' '<<controls_.club_index()<<' '<<static_cast<int>(controls_.stage());
    const auto& m=original_welly_.state();
    out<<' '<<original_welly_active_<<' '<<m.stage<<' '<<m.counter<<' '<<m.angle<<' '<<m.accumulator<<' '<<m.capture_flag<<' '<<m.captured_power<<' '<<m.accuracy_counter;
    if(has_hole()) {
        const auto& h=hole();const auto& b=h.ball_state();
        out<<' '<<b.x_raw<<' '<<b.y_raw<<' '<<b.z_raw<<' '<<b.vx_raw<<' '<<b.vy_raw<<' '<<b.vz_raw<<' '<<b.tick<<' '<<static_cast<int>(h.phase())<<' '<<h.strokes()<<' '<<h.green_mode()<<' '<<h.authored_clock()<<' '<<h.authored_bird_hit()<<' '<<h.bird_hit_age();
    }
    if(game_->has_prng_state()) {const auto& rng=game_->prng_state();out<<' '<<rng.seed0<<' '<<rng.seed1;}
    return out.str();
}
std::string PixteeRoundRuntime::serialize_native_session() const {
    std::ostringstream out;
    out<<"PIXTEE_NATIVE_SESSION_1 "<<initial_seed_.seed0<<' '<<initial_seed_.seed1<<'\n';
    for(auto n:plan_.order())out<<static_cast<unsigned>(n)<<' ';out<<'\n';
    for(auto n:pars_)out<<static_cast<unsigned>(n)<<' ';out<<'\n';
    out<<commands_.size()<<'\n';
    for(const auto& c:commands_) {out<<c.op<<' '<<c.repeat<<' '<<c.args.size();for(auto a:c.args)out<<' '<<a;out<<'\n';}
    out<<"CHECK "<<native_fingerprint()<<'\n';
    return out.str();
}
void PixteeRoundRuntime::restore_native_session(const std::string& checkpoint) {
    if(checkpoint.size()>16*1024*1024)throw std::invalid_argument("session too large");
    std::istringstream input(checkpoint);std::string magic;unsigned seed0,seed1;
    if(!(input>>magic>>seed0>>seed1)||magic!="PIXTEE_NATIVE_SESSION_1"||seed0!=initial_seed_.seed0||seed1!=initial_seed_.seed1)throw std::invalid_argument("native session header mismatch");
    for(auto n:plan_.order()) {unsigned read;if(!(input>>read)||read!=n)throw std::invalid_argument("session course order mismatch");}
    for(auto n:pars_) {unsigned read;if(!(input>>read)||read!=n)throw std::invalid_argument("session pars mismatch");}
    std::size_t count;if(!(input>>count)||count>1000000)throw std::invalid_argument("invalid session command count");
    PixteeRoundRuntime candidate(plan_.order(),pars_,initial_seed_.seed0,initial_seed_.seed1);
    candidate.courses_=courses_;candidate.recording_=false;
    std::uint64_t operations=0;
    for(std::size_t i=0;i<count;++i) {
        Command c{};std::size_t argc;
        if(!(input>>c.op>>c.repeat>>argc)||c.repeat==0||argc>1024||(operations+=c.repeat)>1000000)throw std::invalid_argument("invalid session command");
        c.args.resize(argc);
        for(auto& a:c.args)if(!(input>>a)||a<INT32_MIN||a>INT32_MAX)throw std::invalid_argument("invalid session argument");
        const auto require=[&](std::size_t n){if(c.args.size()!=n)throw std::invalid_argument("session argument count mismatch");};
        for(unsigned repeat=0;repeat<c.repeat;++repeat) {
            switch(c.op) {
            case 'L':require(0);candidate.load_next_hole();break;
            case 'A':require(1);candidate.set_aim_raw(static_cast<std::uint16_t>(c.args[0]));break;
            case 'N':require(1);candidate.nudge_aim_raw(static_cast<std::int32_t>(c.args[0]));break;
            case 'C':require(1);candidate.set_club(static_cast<std::uint16_t>(c.args[0]));break;
            case 'Y':require(1);candidate.cycle_club(static_cast<int>(c.args[0]));break;
            case 'M':require(1);candidate.set_original_player_mode_5a(static_cast<std::uint16_t>(c.args[0]));break;
            case 'F':require(1);candidate.set_verified_flag_object(c.args[0]!=0);break;
            case 'B':require(1);candidate.set_authored_birds_enabled(c.args[0]!=0);break;
            case 'W':require(0);candidate.start_original_welly();break;
            case 'P':require(1);candidate.tick_original_welly(c.args[0]!=0);break;
            case 'I':require(1);candidate.meter_click_raw(static_cast<std::uint16_t>(c.args[0]));break;
            case 'S':require(0);candidate.step_tick();break;
            case 'H':require(0);candidate.acknowledge_hazard();break;
            case 'G':require(0);candidate.acknowledge_special_green();break;
            case 'K':require(0);candidate.cancel_meter();break;
            case 'E':require(0);candidate.commit_scored_hole();break;
            case 'T': {
                if(argc%8)throw std::invalid_argument("invalid session tree geometry");
                std::vector<sensigolf::AuthoredTreeVolume> trees;
                for(std::size_t j=0;j<argc;j+=8)trees.push_back({static_cast<std::uint32_t>(c.args[j]),static_cast<std::int32_t>(c.args[j+1]),static_cast<std::int32_t>(c.args[j+2]),static_cast<std::int32_t>(c.args[j+3]),static_cast<std::int32_t>(c.args[j+4]),static_cast<std::int32_t>(c.args[j+5]),static_cast<std::int32_t>(c.args[j+6]),static_cast<std::int32_t>(c.args[j+7])});
                candidate.set_authored_tree_objects(trees);break;
            }
            default:throw std::invalid_argument("unknown session command");
            }
        }
        candidate.commands_.push_back(std::move(c));
    }
    if(!(input>>magic)||magic!="CHECK")throw std::invalid_argument("session fingerprint missing");
    std::string expected;std::getline(input,expected);
    if(expected!=" "+candidate.native_fingerprint())throw std::invalid_argument("native restore state mismatch");
    input>>std::ws;if(!input.eof())throw std::invalid_argument("trailing session data");
    candidate.recording_=true;
    *this=std::move(candidate); // Publish only after complete native replay and verification.
}
}
