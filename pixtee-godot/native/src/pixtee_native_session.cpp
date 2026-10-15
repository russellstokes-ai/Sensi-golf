#include "pixtee_round_runtime.hpp"
#include <charconv>
#include <limits>
#include <stdexcept>
#include <utility>
namespace pixtee {
void PixteeRoundRuntime::record_command(char op,std::vector<std::int64_t> args) {
    if(!recording_)return;
    if(!commands_.empty() && commands_.back().op==op && commands_.back().args==args && (op=='S'||op=='P'||op=='V')) {
        ++commands_.back().repeat;return;
    }
    commands_.push_back({op,std::move(args),1});
}
namespace {
void number(std::string& out,std::int64_t value) {out+=std::to_string(value);out+=' ';}
struct Reader {
    const std::string& input;std::size_t cursor=0;
    void whitespace() {while(cursor<input.size() && (input[cursor]==' '||input[cursor]=='\n'||input[cursor]=='\r'||input[cursor]=='\t'))++cursor;}
    std::string word() {
        whitespace();const auto start=cursor;
        while(cursor<input.size() && input[cursor]!=' '&&input[cursor]!='\n'&&input[cursor]!='\r'&&input[cursor]!='\t')++cursor;
        if(start==cursor)throw std::invalid_argument("truncated native session");
        return input.substr(start,cursor-start);
    }
    std::int64_t integer() {
        const auto token=word();std::int64_t value=0;
        const auto parsed=std::from_chars(token.data(),token.data()+token.size(),value);
        if(parsed.ec!=std::errc() || parsed.ptr!=token.data()+token.size())throw std::invalid_argument("invalid native integer");
        return value;
    }
};
}
std::string PixteeRoundRuntime::native_fingerprint() const {
    std::string out;
    const auto& r=round_state();
    for(auto value:{static_cast<std::int64_t>(r.current_hole_index),static_cast<std::int64_t>(r.holes_completed),static_cast<std::int64_t>(r.total_strokes),static_cast<std::int64_t>(r.cumulative_par),static_cast<std::int64_t>(r.relative_to_par),static_cast<std::int64_t>(r.round_complete)})number(out,value);
    number(out,controls_.aim_raw());number(out,controls_.club_index());number(out,static_cast<int>(controls_.stage()));
    const auto& m=original_welly_.state();
    number(out,original_welly_active_);number(out,m.stage);number(out,m.counter);number(out,m.angle);number(out,m.accumulator);number(out,m.capture_flag);number(out,m.captured_power);number(out,m.accuracy_counter);
    if(touch_meter_.stage || touch_meter_.ticks) {number(out,touch_meter_.stage);number(out,touch_meter_.position);number(out,touch_meter_.power);number(out,touch_meter_.ticks);number(out,touch_meter_.red_half);number(out,touch_meter_.yellow);}
    if(has_hole()) {
        const auto& h=hole();const auto& b=h.ball_state();
        number(out,b.x_raw);number(out,b.y_raw);number(out,b.z_raw);number(out,b.vx_raw);number(out,b.vy_raw);number(out,b.vz_raw);number(out,b.tick);number(out,static_cast<int>(h.phase()));number(out,h.strokes());number(out,h.green_mode());number(out,h.authored_clock());number(out,h.authored_bird_hit());number(out,h.bird_hit_age());
        if(h.authored_bird().member)number(out,h.authored_bird().member);
        if(h.flag_removed_for_putting()) {number(out,-5207620);number(out,1);}
    }
    if(wind_.enabled) {
        number(out,-5207621);number(out,wind_.toward_raw);number(out,wind_.speed_mph_x10);number(out,wind_.response);
        if(has_hole()) {const auto& a=hole().wind_accumulator();number(out,a.fraction_x);number(out,a.fraction_y);number(out,a.residual_x);number(out,a.residual_y);number(out,a.coordinate_scale);}
    }
    if(game_->has_prng_state()) {const auto& rng=game_->prng_state();number(out,rng.seed0);number(out,rng.seed1);}
    out.pop_back();return out;
}
std::string PixteeRoundRuntime::serialize_native_session() const {
    // Locale-free codec also avoids C++ iostream facet interactions with render drivers.
    std::string out="PIXTEE_NATIVE_SESSION_2 ";number(out,initial_seed_.seed0);number(out,initial_seed_.seed1);out+='\n';
    for(auto n:plan_.order())number(out,n);out+='\n';
    for(auto n:pars_)number(out,n);out+='\n';
    number(out,commands_.size());out+='\n';
    for(const auto& c:commands_) {out+=c.op;out+=' ';number(out,c.repeat);number(out,c.args.size());for(auto a:c.args)number(out,a);out+='\n';}
    out+="CHECK ";out+=native_fingerprint();out+='\n';return out;
}
void PixteeRoundRuntime::restore_native_session(const std::string& checkpoint) {
    if(checkpoint.size()>16*1024*1024)throw std::invalid_argument("session too large");
    Reader input{checkpoint};
    if(input.word()!="PIXTEE_NATIVE_SESSION_2"||input.integer()!=initial_seed_.seed0||input.integer()!=initial_seed_.seed1)throw std::invalid_argument("native session header mismatch");
    for(auto n:plan_.order()) {if(input.integer()!=n)throw std::invalid_argument("session course order mismatch");}
    for(auto n:pars_) {if(input.integer()!=n)throw std::invalid_argument("session pars mismatch");}
    const auto count=input.integer();if(count<0||count>1000000)throw std::invalid_argument("invalid session command count");
    PixteeRoundRuntime candidate(plan_.order(),pars_,initial_seed_.seed0,initial_seed_.seed1);
    candidate.courses_=courses_;candidate.recording_=false;
    std::uint64_t operations=0;
    for(std::size_t i=0;i<count;++i) {
        Command c{};const auto op=input.word();const auto repeats=input.integer();const auto argc=input.integer();
        if(op.size()!=1||repeats<1||repeats>1000000||argc<0||argc>1024||(operations+=repeats)>1000000)throw std::invalid_argument("invalid session command");
        c.op=op[0];c.repeat=static_cast<std::uint32_t>(repeats);
        c.args.resize(argc);
        for(auto& a:c.args) {a=input.integer();if(a<INT32_MIN||a>INT32_MAX)throw std::invalid_argument("invalid session argument");}
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
            case 'J':require(1);candidate.set_flag_removed_for_putting(c.args[0]!=0);break;
            case 'D':require(3);if(c.args[2]!=0 && c.args[2]!=1)throw std::invalid_argument("invalid wind flag");candidate.set_authored_wind(static_cast<std::int32_t>(c.args[0]),static_cast<std::int32_t>(c.args[1]),c.args[2]!=0);break;
            case 'Q':require(1);candidate.set_soft_ground_enabled(c.args[0]!=0);break;
            case 'B':require(1);candidate.set_authored_birds_enabled(c.args[0]!=0);break;
            case 'U':require(0);candidate.start_touch_meter();break;
            case 'V':require(1);candidate.tick_touch_meter(c.args[0]!=0);break;
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
    if(input.word()!="CHECK")throw std::invalid_argument("session fingerprint missing");
    input.whitespace();auto expected=checkpoint.substr(input.cursor);
    while(!expected.empty() && (expected.back()=='\n'||expected.back()=='\r'||expected.back()==' '||expected.back()=='\t'))expected.pop_back();
    if(expected!=candidate.native_fingerprint())throw std::invalid_argument("native restore state mismatch");
    candidate.recording_=true;
    *this=std::move(candidate); // Publish only after complete native replay and verification.
}
}
