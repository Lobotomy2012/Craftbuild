#pragma once

#include <Core/core.hpp>

namespace Craftbuild {
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 tex_coord;
        float tex_id;
        static VkVertexInputBindingDescription get_binding_description() {
            VkVertexInputBindingDescription binding_description{};
            binding_description.binding = 0;
            binding_description.stride = sizeof(Vertex);
            binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return binding_description;
        }

        static std::array<VkVertexInputAttributeDescription, 4> get_attribute_descriptions() {
            std::array<VkVertexInputAttributeDescription, 4> attribute_descriptions{};
            attribute_descriptions[0].binding = 0;
            attribute_descriptions[0].location = 0;
            attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[0].offset = offsetof(Vertex, pos);
            attribute_descriptions[1].binding = 0;
            attribute_descriptions[1].location = 1;
            attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[1].offset = offsetof(Vertex, color);
            attribute_descriptions[2].binding = 0;
            attribute_descriptions[2].location = 2;
            attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attribute_descriptions[2].offset = offsetof(Vertex, tex_coord);
            attribute_descriptions[3].binding = 0;
            attribute_descriptions[3].location = 3;
            attribute_descriptions[3].format = VK_FORMAT_R32_SFLOAT;
            attribute_descriptions[3].offset = offsetof(Vertex, tex_id);
            return attribute_descriptions;
        }

        bool operator==(const Vertex& other) const {
            return pos == other.pos and color == other.color and
                tex_coord == other.tex_coord and tex_id == other.tex_id;
        }
    };
}

namespace std {
    template<> struct hash<Craftbuild::Vertex> {
        size_t operator()(Craftbuild::Vertex const& vertex) const {
            size_t h1 = hash<glm::vec3>()(vertex.pos);
            size_t h2 = hash<glm::vec3>()(vertex.color);
            size_t h3 = hash<glm::vec2>()(vertex.tex_coord);
            size_t h4 = hash<float>()(vertex.tex_id);
            return ((h1 ^ (h2 << 1)) >> 1) ^ ((h3 ^ (h4 << 1)) >> 1);
        }
    };
}