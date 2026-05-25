#include <godot_cpp/classes/engine.hpp>

#include <includes.hpp>
#include <windows.h>

import misc.types;
import misc.format;
import game.core;
import game.environment;
import game.main;
import game.player;
import game.logger;
import game.thread;

using namespace godot;
using namespace craftbuild;

none initialize_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) return;

    log<LogType::VERBOSE>(format{} << "Hello from the DLL! Process ID: " << (uint32)GetCurrentProcessId() << ", Thread ID: " << (uint32)GetCurrentThreadId());
    log<LogType::VERBOSE>(format{} << "Game version: " << full_version);

    ClassDB::register_class<Main>();
    ClassDB::register_class<Player>();
    ClassDB::register_class<Sun>();
    ClassDB::register_class<CraftSky>();
}

none uninitialize_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) return;
}

extern "C" GDExtensionBool GDE_EXPORT craftbuild_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization) {
	ThreadRegistry::register_thread("Main Thread");

    GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_module);
    init_obj.register_terminator(uninitialize_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
