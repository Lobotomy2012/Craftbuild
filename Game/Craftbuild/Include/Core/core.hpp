#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define _USE_MATH_DEFINES
#include <cmath>

#define NOMINMAX
#include <windows.h>
#undef NEAR
#undef FAR
#undef ERROR

#include <FastNoiseLite.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>
#include <functional>
#include <random>
#include <thread>
#include <mutex>
#include <unordered_set>
#include <utility>
#include <fstream>

namespace Craftbuild {
#ifdef _DEBUG
    constexpr bool enable_validation_layers = true;
#else
    constexpr bool enable_validation_layers = false;
#endif

    constexpr bool log_verbose = true;

    inline int64_t make_key(int x, int z) {
        return (static_cast<int64_t>(x) << 32) | static_cast<uint32_t>(z);
    }

    enum class LogLevel {
        VERBOSE,
        INFO,
        WARNING,
        ERROR
    };

    inline void log(LogLevel log_level, const char* message) {
        if constexpr (enable_validation_layers) {
            switch (log_level) {
            case LogLevel::VERBOSE: if constexpr (log_verbose) std::cout << "\033[90m[Verbose][Game] "; break;
            case LogLevel::INFO: std::cout << "\033[96m[Info][Game]\033[36m "; break;
            case LogLevel::WARNING: std::cout << "\033[93m[Warning][Game]\033[33m "; break;
            case LogLevel::ERROR: std::cout << "\033[91m[Error][Game]\033[31m "; break;
            }
            std::cout << message << "\n";
        }
    }
}