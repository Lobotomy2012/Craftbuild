#pragma once

#include <core.hpp>

namespace Craftbuild {
	constexpr float GAME_VERSION = 1.0f;
    constexpr uint32_t WIDTH = 1200;
    constexpr uint32_t HEIGHT = 700;
    constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    const char* GAME_STATE = "Indev";

    const std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const std::vector<const char*> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifdef _DEBUG
    constexpr bool enable_validation_layers = true;
#else
    constexpr bool enable_validation_layers = false;
#endif

    constexpr bool log_verbose = true;
}