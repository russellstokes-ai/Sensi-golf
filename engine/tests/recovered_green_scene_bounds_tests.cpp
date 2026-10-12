#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <vector>
#include "sensigolf/recovered_green_scene_bounds.hpp"
using namespace sensigolf::recovered;
int main(){
    const OriginalGreenSceneObject lower{108,58};
    const OriginalGreenSceneObject upper{212,134};
    assert(original_green_scene_contains(100,50,lower,false));
    assert(original_green_scene_contains(100,50,upper,false));
    assert(!original_green_scene_contains(100,50,{107,58},false));
    assert(!original_green_scene_contains(100,50,{213,58},false));
    assert(!original_green_scene_contains(100,50,{108,135},false));
    assert(original_green_scene_contains(100,50,{16,16},true));
    assert(original_green_scene_contains(100,50,{226,170},true));
    assert(!original_green_scene_contains(100,50,{15,16},true));
    assert(!original_green_scene_contains(100,50,{226,171},true));
    assert(original_green_scene_all_inside(100,50,{lower,upper},false));
    assert(!original_green_scene_all_inside(100,50,
        {lower,{213,134}},false));
    assert(!original_green_scene_all_inside(100,50,{},false));
    assert(original_green_scene_contains(65535,65535,{8,8},false));
    assert(!original_green_scene_contains(65535,65535,{6,8},false));
    return 0;
}
