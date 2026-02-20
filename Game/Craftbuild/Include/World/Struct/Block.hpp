#pragma once

#include <World/Enum/BlockType.hpp>

namespace Craftbuild {
    struct Block {
        static const char* get_texture(BlockType type) {
            switch (type) {
            case BlockType::GRASS:         return "Resource/Craftbuild/Texture/Block/grass_block_top.png";
            case BlockType::DIRT:          return "Resource/Craftbuild/Texture/Block/dirt.png";
            case BlockType::STONE:         return "Resource/Craftbuild/Texture/Block/stone.png";
            case BlockType::DIAMOND_BLOCK: return "Resource/Craftbuild/Texture/Block/diamond_block.png";
            case BlockType::WATER:         return "Resource/Craftbuild/Texture/Block/water_overlay.png";
            case BlockType::SAND:          return "Resource/Craftbuild/Texture/Block/sand.png";
            case BlockType::WOOD:          return "Resource/Craftbuild/Texture/Block/oak_log.png";
            case BlockType::LEAVES:        return "Resource/Craftbuild/Texture/Block/oak_leaves.png";
            case BlockType::BEDROCK:       return "Resource/Craftbuild/Texture/Block/bedrock.png";
            case BlockType::GRAVEL:        return "Resource/Craftbuild/Texture/Block/gravel.png";
            case BlockType::SNOW:          return "Resource/Craftbuild/Texture/Block/snow.png";
            case BlockType::GLASS:         return "Resource/Craftbuild/Texture/Block/glass.png";
            case BlockType::GRASS_PLANT:   return "Resource/Craftbuild/Texture/Block/bedrock.png";
            }
            return "Resource/Craftbuild/Texture/Block/bedrock.png";
        }

        static float get_texture_index(BlockType type) {
            return (float)type;
        }
    };
}