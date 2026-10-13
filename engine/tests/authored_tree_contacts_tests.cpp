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
    // A grazing native deflection can still aim into the same trunk. Event
    // dedupe must never allow that subsequent tick to penetrate the volume.
    const std::vector<AuthoredTreeVolume> grazing{{7,100*q,100*q,2*q,10*q,12*q,10*q,40*q}};
    int grazing_hits=0;
    for (int seed=1;seed<=64;++seed) {
        ClassicShotModel model;
        recovered::OriginalPrng16 rng{static_cast<std::uint16_t>(seed),0xabcd};
        model.reset(); model.set_surface_context({3,0,0});
        model.set_ball_origin(98*q+q/20,90*q);
        model.set_authored_tree_context(&grazing,0,0,false);
        ShotInput input{}; input.club_index=12;input.lie_index=4;
        input.power_tick=105;input.accuracy_tick=63;
        model.begin_shot(input);
        model.set_original_interaction_context(&rng,false,0,0);
        for(int tick=0;tick<4096 && model.shot_active();++tick) {
            model.step();
            if(model.presentation_contact().tree_id)++grazing_hits;
            const auto dx=std::int64_t(model.state().x_raw)-100*q;
            const auto dy=std::int64_t(model.state().y_raw)-100*q;
            assert(dx*dx+dy*dy>=std::int64_t(2*q)*(2*q));
        }
        assert(!model.shot_active());
    }
    assert(grazing_hits>0);
    const AuthoredTreeVolume low_canopy{2,15*q,0,2*q,10*q,12*q,0,40*q};
    assert(authored_tree_contact({low_canopy},{0,0,5*q},{20*q,0,5*q}).kind==AuthoredTreeContactKind::Canopy);
    const AuthoredTreeVolume far_trunk{2,15*q,0,2*q,10*q,2*q,10*q,40*q};
    const AuthoredTreeVolume near_trunk{3,5*q,0,2*q,10*q,2*q,10*q,40*q};
    assert(authored_tree_contact({far_trunk,near_trunk},{0,0,0},{20*q,0,0}).tree_id==3);
    assert(authored_tree_contact({near_trunk,far_trunk},{0,0,0},{20*q,0,0}).tree_id==3);
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
