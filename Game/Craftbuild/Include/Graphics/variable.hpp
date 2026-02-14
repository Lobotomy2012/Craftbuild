#pragma once

#include <core.hpp>

namespace Craftbuild {
	const float GAME_VERSION = 1.0f;
    const char* GAME_STATE = "Indev";
    const uint32_t WIDTH = 1200;
    const uint32_t HEIGHT = 700;
    const int MAX_FRAMES_IN_FLIGHT = 2;

    const std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const std::vector<const char*> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifdef _DEBUG
    const bool enable_validation_layers = true;
#else
    const bool enable_validation_layers = false;
#endif

    const bool log_verbose = true;
}