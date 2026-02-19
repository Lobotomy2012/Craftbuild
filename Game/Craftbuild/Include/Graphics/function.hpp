#pragma once

#include <core.hpp>

namespace Craftbuild {
    inline VkResult create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* p_create_info, const VkAllocationCallbacks* p_allocator, VkDebugUtilsMessengerEXT* p_debug_messenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, p_create_info, p_allocator, p_debug_messenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    inline void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* p_allocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debug_messenger, p_allocator);
        }
    }

    inline int64_t make_key(int x, int z) {
        return (static_cast<int64_t>(x) << 32) | static_cast<uint32_t>(z);
    }

    enum class LogLevel {
        VERBOSE,
        INFO,
        WARNING,
        ERROR
    };

    void log(LogLevel log_level, const char* message) {
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