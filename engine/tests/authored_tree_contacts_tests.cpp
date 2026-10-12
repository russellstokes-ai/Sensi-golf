#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include "sensigolf/authored_tree_contacts.hpp"
#include "sensigolf/classic_shot_model.hpp"
using namespace sensigolf;
int main() {
    constexpr int q=65536;
    AuthoredTreeVolume tree{1,0,10*q,2*q,10*q,12*q,10*q,40*q};
    const std::vector<AuthoredTreeVolume> trees{tree};
    assert(authored_tree_contact(trees,{0,0,0},{0,20*q,0}).kind==AuthoredTreeContactKind::Trunk);
    assert(authored_tree_contact(trees,{-20*q,10*q,20*q},{20*q,10*q,20*q}).kind==AuthoredTreeContactKind::Canopy);
    assert(authored_tree_contact(trees,{-20*q,10*q,41*q},{20*q,10*q,41*q}).kind==AuthoredTreeContactKind::None);
    assert(authored_tree_contact(trees,{3*q,0,0},{3*q,20*q,0}).kind==AuthoredTreeContactKind::None);
    assert(authored_tree_contact({}, {0,0,0},{0,20*q,0}).kind==AuthoredTreeContactKind::None);
    ClassicShotModel a,b;
    recovered::OriginalPrng16 ra{0x1234,0xabcd},rb=ra;
    for(auto* model:{&a,&b}) {
        model->reset();
        model->set_surface_context({3,0,0});
        model->set_ball_origin(0,0);
        model->set_authored_tree_context(&trees,0,0,false);
        ShotInput input{}; input.club_index=12; input.lie_index=4;
        input.power_tick=105; input.accuracy_tick=63;
        model->begin_shot(input);
    }
    a.set_original_interaction_context(&ra,false,0,0);
    b.set_original_interaction_context(&rb,false,0,0);
    int hits=0;
    for(int i=0;i<4096 && a.shot_active();++i) {
        a.step(); b.step();
        assert(a.state().x_raw==b.state().x_raw && a.state().y_raw==b.state().y_raw);
        assert(ra.seed0==rb.seed0 && ra.seed1==rb.seed1);
        if(a.presentation_contact().tree_id) ++hits;
    }
    assert(hits==1 && !a.shot_active());
    // Authored response consumes the existing native PRNG, never a new RNG.
    assert(ra.seed0!=0x1234 || ra.seed1!=0xabcd);
}
