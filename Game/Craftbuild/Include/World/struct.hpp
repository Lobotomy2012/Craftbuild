#pragma once

#include <core.hpp>
#include <World/enum.hpp>

namespace Craftbuild {
    struct Block {
        BlockType type = BlockType::AIR;
        bool visible = false;

        static const char* get_texture(BlockType type) {
            switch (type) {
            case BlockType::GRASS:         return "Resource/Craftbuild/Texture/Block/grass_block_top.png";
            case BlockType::DIRT:          return "Resource/Craftbuild/Texture/Block/dirt.png";
            case BlockType::STONE:         return "Resource/Craftbuild/Texture/Block/stone.png";
            case BlockType::DIAMOND_BLOCK: return "Resource/Craftbuild/Texture/Block/diamond_block.png";
            case BlockType::WATER:         return "Resource/Craftbuild/Texture/Block/water_still.png";
            case BlockType::SAND:          return "Resource/Craftbuild/Texture/Block/sand.png";
            case BlockType::WOOD:          return "Resource/Craftbuild/Texture/Block/oak_log.png";
            case BlockType::LEAVES:        return "Resource/Craftbuild/Texture/Block/oak_leaves.png";
            case BlockType::BEDROCK:       return "Resource/Craftbuild/Texture/Block/bedrock.png";
            case BlockType::GRAVEL:        return "Resource/Craftbuild/Texture/Block/gravel.png";
            case BlockType::SNOW:          return "Resource/Craftbuild/Texture/Block/snow.png";
            case BlockType::GRASS_PLANT:   return "Resource/Craftbuild/Texture/Block/bedrock.png";
            }
            return "Resource/Craftbuild/Texture/Block/bedrock.png";
        }

        static float get_texture_index(BlockType type) {
            switch (type) {
            case BlockType::GRASS:        return 0.0f;
            case BlockType::DIRT:         return 1.0f;
            case BlockType::STONE:        return 2.0f;
            case BlockType::DIAMOND_BLOCK:return 3.0f;
            case BlockType::WATER:        return 4.0f;
            case BlockType::SAND:         return 5.0f;
            case BlockType::WOOD:         return 6.0f;
            case BlockType::LEAVES:       return 7.0f;
            case BlockType::BEDROCK:      return 8.0f;
            case BlockType::GRAVEL:       return 9.0f;
            case BlockType::SNOW:         return 10.0f;
            case BlockType::GRASS_PLANT:  return 11.0f;
            }
            return 8.0f;
        }
    };

    struct Vertex;
    struct Chunk {
        int x, z;
        BlockType blocks[16][383][16]; // 16x383x16 blocks
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        LODLevel lod_level;

        void generate(int chunk_x, int chunk_z) {
            x = chunk_x;
            z = chunk_z;
            vertices.clear();
            indices.clear();

            for (int x = 0; x < 16; x++) {
                for (int y = 0; y < 383; y++) {
                    for (int z = 0; z < 16; z++) {
                        blocks[x][y][z] = BlockType::AIR;
                    }
                }
            }
        }

        void clear_mesh() {
            vertices.clear();
            indices.clear();
            vertices.shrink_to_fit();
            indices.shrink_to_fit();
        }
    };
}