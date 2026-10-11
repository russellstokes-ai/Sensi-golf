#include "pixtee_gameplay.hpp"
#include "pixtee_round_bridge.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/godot.hpp"

using namespace godot;

void initialize_pixtee(ModuleInitializationLevel level) {
    if (level != MODULE_INITIALIZATION_LEVEL_SCENE) return;
    ClassDB::register_class<PixteeGameplay>();
    ClassDB::register_class<PixteeRoundBridge>();
}
void uninitialize_pixtee(ModuleInitializationLevel level) {
    (void)level;
}
extern "C" {
GDExtensionBool GDE_EXPORT pixtee_library_init(
    GDExtensionInterfaceGetProcAddress get_proc_address,
    const GDExtensionClassLibraryPtr library,
    GDExtensionInitialization* initialization) {
    GDExtensionBinding::InitObject init(get_proc_address, library, initialization);
    init.register_initializer(initialize_pixtee);
    init.register_terminator(uninitialize_pixtee);
    init.set_minimum_library_initialization_level(
        MODULE_INITIALIZATION_LEVEL_SCENE);
    return init.init();
}
}
