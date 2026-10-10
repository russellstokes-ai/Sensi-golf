#include <jni.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "sensigolf/classic_game_session.hpp"
#include "sensigolf/classic_mobile_controls.hpp"

namespace {

using namespace sensigolf;
constexpr std::array<std::uint8_t,18> kOrder{{
    42,50,58,38,70,44,64,48,25,52,40,60,68,17,66,56,46,72}};
constexpr std::array<std::uint8_t,18> kPars{{
    4,4,3,4,3,4,5,5,4,5,4,5,5,3,5,4,3,4}};
constexpr int kSampleStep = 4;
std::unique_ptr<ClassicGameSession> game;
std::unique_ptr<ClassicCourseResources> drawing_course;
ClassicMobileControls controls;
std::string root, status = "Select an extracted original v1.014 course folder.";
std::vector<jint> map_pixels;
int map_w = 0, map_h = 0, resource_id = -1;

std::string read_string(JNIEnv* env,jstring value) {
    if(!value) throw std::invalid_argument("missing resource folder");
    const char* chars=env->GetStringUTFChars(value,nullptr);
    if(!chars) throw std::runtime_error("Android string conversion failed");
    std::string result(chars);
    env->ReleaseStringUTFChars(value,chars);
    return result;
}
void report(JNIEnv* env,const std::exception& e) {
    jclass cls=env->FindClass("java/lang/IllegalStateException");
    if(cls) env->ThrowNew(cls,e.what());
}
std::vector<std::uint8_t> file(const std::string& filename) {
    std::ifstream input(filename,std::ios::binary);
    if(!input) throw std::runtime_error("Required original course file missing: " +
        filename.substr(filename.find_last_of('/')+1));
    std::vector<std::uint8_t> result(
        (std::istreambuf_iterator<char>(input)),std::istreambuf_iterator<char>());
    if(result.empty() || result.size()>16u*1024u*1024u)
        throw std::runtime_error("Original resource invalid or too large");
    return result;
}
std::string named(const char* prefix,int id,const char* suffix) {
    std::ostringstream out;
    out<<root<<"/"<<prefix<<std::setw(2)<<std::setfill('0')<<id<<"."<<suffix;
    return out.str();
}
ClassicCourseResources course_for(int id) {
    return ClassicCourseResources(
        file(named("MAPM",id,"MAP")),file(named("MAPM",id,"SPT")),
        file(root+"/MAPI01.RAW"),file(root+"/MAPI02.RAW"),
        file(named("MAPS",id,"MAP")));
}
ClassicHolePlan plan() {
    std::array<std::uint8_t,kClassicParTableSize> pars{};
    for(std::size_t i=0;i<kOrder.size();++i) pars[kOrder[i]]=kPars[i];
    return ClassicHolePlan(kOrder,pars);
}
bool playing() { return game && game->has_active_hole(); }
jint colour(const ResolvedCourseSurface& s) {
    switch(s.landing_code) {
    case 1: case 8: case 9: case 10: return static_cast<jint>(0xff9abc58u);
    case 2: return static_cast<jint>(0xff81ad49u);
    case 3: return static_cast<jint>(0xff76a94du);
    case 4: return static_cast<jint>(0xff608d3cu);
    case 5: return static_cast<jint>(0xff436d32u);
    case 6: return static_cast<jint>(0xff34572bu);
    case 7: return static_cast<jint>(0xffd8be79u);
    case 35: return s.name.find("WATER")!=std::string_view::npos
        ? static_cast<jint>(0xff388ca7u):static_cast<jint>(0xff303a2eu);
    default: return static_cast<jint>(0xff34432au);
    }
}
void build_map(const ClassicCourseResources& course) {
    const int width=static_cast<int>(course.map_width())*16;
    const int height=static_cast<int>(course.map_height())*8;
    map_w=std::min(512,std::max(1,(width+kSampleStep-1)/kSampleStep));
    map_h=std::min(512,std::max(1,(height+kSampleStep-1)/kSampleStep));
    map_pixels.assign(static_cast<std::size_t>(map_w)*map_h,
        static_cast<jint>(0xff283628u));
    for(int y=0;y<map_h;++y) {
        for(int x=0;x<map_w;++x) {
            try {
                map_pixels[static_cast<std::size_t>(y)*map_w+x]=colour(
                    course.resolve_integer_position(
                        static_cast<std::int16_t>(x*kSampleStep),
                        static_cast<std::int16_t>(y*kSampleStep)));
            } catch(const std::exception&) {
                // Unmapped border; never used to calculate physics.
            }
        }
    }
}
void load_next() {
    if(!game) throw std::logic_error("engine not initialized");
    const auto request=game->resource_request();
    if(!request) {
        status="Finished 18 original holes";
        map_pixels.clear();
        drawing_course.reset();
        return;
    }
    auto data=course_for(request->resource_id);
    auto render=std::make_unique<ClassicCourseResources>(data);
    build_map(*render);
    game->load_current_hole(request->resource_id,std::move(data));
    drawing_course=std::move(render);
    resource_id=request->resource_id;
    controls.reset();
    controls.sync_hole(game->active_hole());
    status="Original C++ physics engine running";
}
std::string snapshot() {
    std::ostringstream out;
    out<<std::fixed<<std::setprecision(3);
    out<<"{\"loaded\":"<<(playing()?"true":"false");
    if(!game) { out<<",\"ready\":false,\"complete\":false}";return out.str(); }
    const auto& round=game->round_state();
    out<<",\"roundIndex\":"<<round.current_hole_index
       <<",\"totalStrokes\":"<<round.total_strokes
       <<",\"totalPar\":"<<round.cumulative_par
       <<",\"complete\":"<<(round.round_complete?"true":"false")
       <<",\"courseId\":"<<resource_id;
    if(playing()) {
        const auto& h=game->active_hole();
        const auto& b=h.ball_state();
        const bool moving=h.phase()==HoleSessionPhase::ShotActive;
        double x=static_cast<double>(moving?b.x_raw:h.ball_x_raw())/65536.0;
        double y=static_cast<double>(moving?b.y_raw:h.ball_y_raw())/65536.0;
        if(h.green_mode() && drawing_course) {
            const auto region=drawing_course->green_region();
            if(region) { x=x/2.0+region->origin_x; y=y/2.0+region->origin_y; }
        }
        const auto cup=h.hole_position();
        out<<",\"x\":"<<x<<",\"y\":"<<y
           <<",\"height\":"<<(moving?std::max(0.0,static_cast<double>(b.z_raw)/65536.0):0)
           <<",\"cupX\":"<<cup.x<<",\"cupY\":"<<cup.y
           <<",\"phase\":"<<static_cast<int>(h.phase())
           <<",\"strokes\":"<<h.strokes()
           <<",\"distance\":"<<h.distance_to_hole()
           <<",\"club\":"<<controls.club_index()
           <<",\"aim\":"<<controls.aim_raw()
           <<",\"meterStage\":"<<static_cast<int>(controls.stage())
           <<",\"ready\":"<<(controls.enabled()?"true":"false")
           <<",\"mapW\":"<<map_w<<",\"mapH\":"<<map_h
           <<",\"green\":"<<(h.green_mode()?"true":"false");
    }
    out<<"}";
    return out.str();
}
void tick(int count) {
    if(!playing()) return;
    for(int i=0;i<std::min(10,std::max(0,count));++i) {
        auto& h=game->active_hole();
        switch(h.phase()) {
        case HoleSessionPhase::ShotActive:
        case HoleSessionPhase::HazardStopped:
        case HoleSessionPhase::CupTerminal:
        case HoleSessionPhase::ReadyForShot:
            h.step();
            break;
        case HoleSessionPhase::HazardRecovered:
            h.acknowledge_hazard_recovery();
            break;
        case HoleSessionPhase::SpecialGreenStopped:
            if(h.special_green_pause_remaining()) h.step();
            else status="Special green continuation not recovered — shot gated";
            break;
        case HoleSessionPhase::UnsupportedTerrain:
            status="Original rule unsupported — physics stopped safely";
            break;
        case HoleSessionPhase::HoleScored:
            break;
        }
        if(h.phase()==HoleSessionPhase::HoleScored) {
            game->commit_scored_hole();
            load_next();
            break;
        }
        controls.sync_hole(h);
    }
}
void click(int meter_raw) {
    if(!playing()) return;
    controls.sync_hole(game->active_hole());
    controls.meter_click(static_cast<std::uint16_t>(meter_raw));
    if(controls.queued_shot()) controls.dispatch_to(game->active_hole());
}

} // namespace

extern "C" JNIEXPORT void JNICALL
Java_com_russellstokes_sensigolf_NativeBridge_nativeLoad(
    JNIEnv* env,jclass,jstring directory) {
    try {
        root=read_string(env,directory);
        game=std::make_unique<ClassicGameSession>(plan());
        drawing_course.reset();
        map_pixels.clear();
        load_next();
    } catch(const std::exception& e) { game.reset();report(env,e); }
}
extern "C" JNIEXPORT jstring JNICALL
Java_com_russellstokes_sensigolf_NativeBridge_nativeSnapshot(JNIEnv* env,jclass) {
    try { return env->NewStringUTF(snapshot().c_str()); }
    catch(const std::exception& e) { report(env,e);return nullptr; }
}
extern "C" JNIEXPORT jstring JNICALL
Java_com_russellstokes_sensigolf_NativeBridge_nativeStatus(JNIEnv* env,jclass) {
    return env->NewStringUTF(status.c_str());
}
extern "C" JNIEXPORT jintArray JNICALL
Java_com_russellstokes_sensigolf_NativeBridge_nativePixels(JNIEnv* env,jclass) {
    jintArray array=env->NewIntArray(static_cast<jsize>(map_pixels.size()));
    if(array && !map_pixels.empty())
        env->SetIntArrayRegion(array,0,static_cast<jsize>(map_pixels.size()),map_pixels.data());
    return array;
}
extern "C" JNIEXPORT void JNICALL
Java_com_russellstokes_sensigolf_NativeBridge_nativeStep(JNIEnv* env,jclass,jint count) {
    try { tick(count); } catch(const std::exception& e) { status=e.what();report(env,e); }
}
extern "C" JNIEXPORT void JNICALL
Java_com_russellstokes_sensigolf_NativeBridge_nativeAim(JNIEnv* env,jclass,jint delta) {
    try { controls.nudge_aim(delta); }
    catch(const std::exception& e) { report(env,e); }
}
extern "C" JNIEXPORT void JNICALL
Java_com_russellstokes_sensigolf_NativeBridge_nativeClub(JNIEnv* env,jclass,jint delta) {
    try { controls.cycle_club(delta); }
    catch(const std::exception& e) { report(env,e); }
}
extern "C" JNIEXPORT void JNICALL
Java_com_russellstokes_sensigolf_NativeBridge_nativeClick(JNIEnv* env,jclass,jint reading) {
    try {
        if(reading<0 || reading>105) throw std::invalid_argument("meter outside 0..105");
        click(reading);
    } catch(const std::exception& e) { report(env,e); }
}
extern "C" JNIEXPORT void JNICALL
Java_com_russellstokes_sensigolf_NativeBridge_nativeCancel(JNIEnv*,jclass) {
    controls.cancel_meter();
}
extern "C" JNIEXPORT void JNICALL
Java_com_russellstokes_sensigolf_NativeBridge_nativeReset(JNIEnv* env,jclass) {
    try {
        if(!root.empty()) {
            game=std::make_unique<ClassicGameSession>(plan());
            load_next();
        }
    } catch(const std::exception& e) { report(env,e); }
}
