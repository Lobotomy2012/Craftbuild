#pragma once

#include <core.hpp>
#include <World/enum.hpp>
#include <World/variable.hpp>
#include <Graphics/Struct/Vertex.hpp>

namespace Craftbuild {
    struct Chunk {
        int x, z;
        BlockType blocks[CHUNK_SIZE][WORLD_HEIGHT][CHUNK_SIZE]; // 16x383x16 blocks
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

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

        friend std::ostream& operator<<(std::ostream& os, const Chunk& other) {
            const char magic[4] = { 'C','B','C','H' };
            os.write(magic, 4);
            int32_t cx = other.x;
            int32_t cz = other.z;
            os.write(reinterpret_cast<const char*>(&cx), sizeof(cx));
            os.write(reinterpret_cast<const char*>(&cz), sizeof(cz));
            os.write(reinterpret_cast<const char*>(other.blocks), sizeof(other.blocks));
            return os;
        }

        friend std::istream& operator>>(std::istream& is, Chunk& other) {
            std::streampos start_pos = is.tellg();
            char magic[4];
            if (!is.read(magic, 4)) return is;

            const char expected[4] = { 'C','B','C','H' };
            if (std::memcmp(magic, expected, 4) == 0) {
                int32_t cx = 0, cz = 0;
                is.read(reinterpret_cast<char*>(&cx), sizeof(cx));
                is.read(reinterpret_cast<char*>(&cz), sizeof(cz));
                // Read raw blocks
                is.read(reinterpret_cast<char*>(other.blocks), sizeof(other.blocks));
                other.x = cx;
                other.z = cz;
                other.vertices.clear();
                other.indices.clear();
            }
            return is;
        }
    };
}