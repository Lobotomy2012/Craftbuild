#pragma once

#include <core.hpp>
#include <World/enum.hpp>
#include <World/variable.hpp>

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
            case BlockType::GLASS:         return "Resource/Craftbuild/Texture/Block/glass.png";
            case BlockType::GRASS_PLANT:   return "Resource/Craftbuild/Texture/Block/bedrock.png";
            }
            return "Resource/Craftbuild/Texture/Block/bedrock.png";
        }

        static float get_texture_index(BlockType type) {
            return (float)type;
        }
    };

    struct Vertex;
    struct Chunk {
        int x, z;
        BlockType blocks[CHUNK_SIZE][WORLD_HEIGHT][CHUNK_SIZE]; // 16x383x16 blocks
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        LODLevel lod_level;

        void generate(int chunk_x, int chunk_z) {
            x = chunk_x;
            z = chunk_z;
            vertices.clear();
            indices.clear();

            for (int x = 0; x < CHUNK_SIZE; x++) {
                for (int y = 0; y < WORLD_HEIGHT; y++) {
                    for (int z = 0; z < CHUNK_SIZE; z++) {
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