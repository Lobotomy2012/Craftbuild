#pragma once

#include <Core/core.hpp>

namespace Craftbuild {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;
        bool is_complete() {
            return graphics_family.has_value() and present_family.has_value();
        }
    };
}