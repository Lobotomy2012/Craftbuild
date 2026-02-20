#pragma once

#include <Core/core.hpp>
#include <World/Enum/BlockType.hpp>
#include <World/Enum/BiomeType.hpp>
#include <World/variable.hpp>
#include <World/Struct/Block.hpp>
#include <World/Struct/Chunk.hpp>
#include <World/Class/Frustum.hpp>

namespace Craftbuild {
    class World {
        FastNoiseLite continentalness, erosion, peaks_valleys, weirdness, temperature, humidity, cave_noise, tree_noise;
        int seed;

        double get_density(int x, int y, int z) {
            // ===== 2D terrain shape =====
            double cont = (continentalness.GetNoise(x * 0.001, z * 0.001) + 1.0) * 0.5;
            cont = std::pow(cont, 1.3);

            double erosion_val = erosion.GetNoise(x * 0.002, z * 0.002);

            double peak = peaks_valleys.GetNoise(x * 0.003, z * 0.003);
            peak = 1.0 - std::abs(peak);
            peak *= peak;

            double base_height = 64.0 + cont * 80.0;

            double mountain = peak * cont * 120.0;
            mountain *= (1.0 - (erosion_val + 1.0) * 0.5);

            double terrain_height = base_height + mountain;

            // ===== convert to density =====
            double height_density = terrain_height - y;

            // ===== 3D cave =====
            double cheese = cave_noise.GetNoise(x * 0.03, y * 0.03, z * 0.03);
            double noodle = cave_noise.GetNoise(x * 0.08, y * 0.08, z * 0.08);

            double cave = std::max(cheese * 1.2, noodle);

            double depth_factor = std::clamp(1.0 - (double)y / WORLD_HEIGHT, 0.0, 1.0);

            return height_density - cave * 30.0 * depth_factor;
        }

        double get_height(int x, int z) {
            double cont = (continentalness.GetNoise(x * 0.001, z * 0.001) + 1.0) * 0.5;
            cont = std::pow(cont, 1.3);
            double ero = erosion.GetNoise(x * 0.002, z * 0.002);
            double peak = peaks_valleys.GetNoise(x * 0.003, z * 0.003);
            peak = 1.0 - std::abs(peak);
            peak = peak * peak;
            double w = weirdness.GetNoise(x * 0.004, z * 0.004);
            double base_height = 64.0 + cont * 80.0;
            double mountain = peak * cont * 120.0;
            double erosion_factor = (ero + 1.0) * 0.5;
            mountain *= (1.0 - erosion_factor);
            double final_height = base_height + mountain;

            return std::clamp(final_height, 0.0, 320.0);
        }

        BiomeType get_biome(int x, int z) {
            double temp = temperature.GetNoise(x * 0.0005, z * 0.0005) * 2.0 - 1.0;
            double humid = humidity.GetNoise(x * 0.0007, z * 0.0007) * 2.0 - 1.0;
            double height = get_height(x, z);

            if (height < 62) return BiomeType::OCEAN;
            if (height < 65) return BiomeType::BEACH;

            if (temp < -0.4) return BiomeType::TAIGA;
            if (temp > 0.6) {
                if (humid < -0.3) return BiomeType::DESERT;
                if (humid > 0.5) return BiomeType::JUNGLE;
                return BiomeType::SAVANNA;
            }
            if (humid > 0.6) return BiomeType::SWAMP;
            if (height > 140) return BiomeType::MOUNTAINS;

            double forest = tree_noise.GetNoise(x * 0.002, z * 0.002);
            if (forest > 0.4 and humid > 0.2) return BiomeType::FOREST;

            return BiomeType::PLAINS;
        }

        bool should_generate_cave(int x, int y, int z) {
            if (y < 10 or y > 100) return false;

            // Cheese caves (big round holes)
            double cheese = cave_noise.GetNoise(x * 0.03, y * 0.03, z * 0.03);
            // Noodle caves (thin tunnels)
            double noodle = cave_noise.GetNoise(x * 0.08, y * 0.08, z * 0.08);
            // Spaghetti caves (winding tunnels)
            double spaghetti = cave_noise.GetNoise(x * 0.05, y * 0.05, z * 0.05);

            double density = 0.65 - (y / 256.0) * 0.3;
            return (cheese > density * 1.2) or (noodle > density * 0.9) or (spaghetti > density * 1.0);
        }

        void generate_terrain(Chunk& chunk) {
            for (int lx = 0; lx < CHUNK_SIZE; ++lx) {
                for (int lz = 0; lz < CHUNK_SIZE; ++lz) {
                    int wx = chunk.x * CHUNK_SIZE + lx;
                    int wz = chunk.z * CHUNK_SIZE + lz;

                    for (int ly = 0; ly < WORLD_HEIGHT; ++ly) {
                        int wy = ly;
                        double density = get_density(wx, wy, wz);
                        if (wy <= 4) {
                            if (wy <= rand() % 5)
                                chunk.blocks[lx][ly][lz] = BlockType::BEDROCK;
                            else
                                chunk.blocks[lx][ly][lz] = BlockType::STONE;

                            continue;
                        }

                        if (density > 0.0) {
                            chunk.blocks[lx][ly][lz] = BlockType::STONE;
                        }
                        else if (wy <= 62) {
                            chunk.blocks[lx][ly][lz] = BlockType::WATER;
                        }
                        else {
                            chunk.blocks[lx][ly][lz] = BlockType::AIR;
                        }
                    }
                }
            }
        }

        void generate_caves(Chunk& chunk) {
            for (int lx = 0; lx < CHUNK_SIZE; ++lx) {
                for (int ly = 0; ly < WORLD_HEIGHT; ++ly) {
                    for (int lz = 0; lz < CHUNK_SIZE; ++lz) {
                        int wx = chunk.x * CHUNK_SIZE + lx;
                        int wz = chunk.z * CHUNK_SIZE + lz;
                        BlockType& block = chunk.blocks[lx][ly][lz];
                        if (block != BlockType::AIR and block != BlockType::WATER and block != BlockType::BEDROCK) {
                            if (should_generate_cave(wx, ly, wz)) {
                                block = BlockType::AIR;
                            }
                        }
                    }
                }
            }
        }

        void generate_trees(Chunk& chunk) {
            std::mt19937 rng(seed + chunk.x * 73856093 + chunk.z * 19349663);
            std::uniform_int_distribution<> height_dist(5, 12);
            std::uniform_real_distribution<> rand(0.0, 1.0);

            for (int lx = 0; lx < 16; ++lx) {
                for (int lz = 0; lz < 16; ++lz) {
                    int wx = chunk.x * 16 + lx;
                    int wz = chunk.z * 16 + lz;
                    BiomeType biome = get_biome(wx, wz);

                    double density = 0.0;
                    switch (biome) {
                    case BiomeType::FOREST: density = 0.08; break;
                    case BiomeType::JUNGLE: density = 0.12; break;
                    case BiomeType::TAIGA:  density = 0.05; break;
                    case BiomeType::PLAINS: density = 0.015; break;
                    default: density = 0.0;
                    }
                    if (rand(rng) > density) continue;

                    int ground_y = -1;
                    for (int ly = 382; ly >= 0; --ly) {
                        if (chunk.blocks[lx][ly][lz] == BlockType::GRASS or
                            chunk.blocks[lx][ly][lz] == BlockType::DIRT or
                            chunk.blocks[lx][ly][lz] == BlockType::SAND) {
                            ground_y = ly;
                            break;
                        }
                    }
                    if (ground_y < 4 or ground_y > 200) continue;

                    int height = height_dist(rng);
                    for (int h = 1; h <= height; ++h) {
                        int ty = ground_y + h;
                        if (ty >= 383) break;
                        chunk.blocks[lx][ty][lz] = BlockType::WOOD;
                    }

                    int leaf_y = ground_y + height;
                    int radius = (biome == BiomeType::JUNGLE) ? 5 : 4;
                    for (int dx = -radius; dx <= radius; ++dx) {
                        for (int dz = -radius; dz <= radius; ++dz) {
                            for (int dy = -radius / 2; dy <= radius; ++dy) {
                                int px = lx + dx, pz = lz + dz, py = leaf_y + dy;
                                if (px < 0 or px >= 16 or pz < 0 or pz >= 16 or py < 0 or py >= 383) continue;

                                double dist = std::sqrt(dx * dx + dy * dy * 0.7 + dz * dz);
                                if (dist <= radius + rand(rng) * 0.6 - 0.3) {
                                    if (chunk.blocks[px][py][pz] == BlockType::AIR) {
                                        chunk.blocks[px][py][pz] = BlockType::LEAVES;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        void decorate_surface(Chunk& chunk) {
            for (int lx = 0; lx < CHUNK_SIZE; ++lx) {
                for (int lz = 0; lz < CHUNK_SIZE; ++lz) {
                    int wx = chunk.x * CHUNK_SIZE + lx;
                    int wz = chunk.z * CHUNK_SIZE + lz;

                    BiomeType biome = get_biome(wx, wz);

                    int surface_depth = 0;

                    for (int ly = WORLD_HEIGHT - 1; ly >= 0; --ly) {
                        BlockType& b = chunk.blocks[lx][ly][lz];

                        if (b == BlockType::AIR) {
                            surface_depth = 0;
                            continue;
                        }

                        if (b == BlockType::STONE) {
                            if (surface_depth == 0) {
                                switch (biome) {
                                case BiomeType::DESERT:
                                case BiomeType::BEACH: b = BlockType::SAND; break;
                                default: b = BlockType::GRASS;
                                }
                            }
                            else if (surface_depth < 4) {
                                b = BlockType::DIRT;
                            }

                            surface_depth++;
                        }
                    }
                }
            }
        }

        void set_seed(FastNoiseLite& noise, int seed, float octaves, float frequency) {
            noise.SetSeed(seed);
            noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
            noise.SetFractalType(FastNoiseLite::FractalType_FBm);
            noise.SetFractalOctaves(octaves);
            noise.SetFrequency(frequency);
        }

    public:
        World(int seed = 123456) : seed(seed),
            continentalness(seed + 0),
            erosion(seed + 1),
            peaks_valleys(seed + 2),
            weirdness(seed + 3),
            temperature(seed + 4),
            humidity(seed + 5),
            cave_noise(seed + 6),
            tree_noise(seed + 7) {}

        void generate_chunk(Chunk& chunk) {
            generate_terrain(chunk);
            decorate_surface(chunk);
            generate_trees(chunk);
        }

        void set_seed(int seed) {
            this->seed = seed;
			set_seed(continentalness, seed, 5, 0.003f);
            set_seed(erosion, seed << 1, 3, 0.01f);
            set_seed(peaks_valleys, seed << 2, 3, 0.01f);
            set_seed(weirdness, seed << 3, 5, 0.003f);
            set_seed(temperature, seed << 4, 5, 0.003f);
            set_seed(humidity, seed << 5, 3, 0.01f);
            set_seed(cave_noise, seed << 6, 3, 0.01f);
            set_seed(tree_noise, seed << 6, 5, 0.003f);
        }

        int get_seed() const {
            return seed;
        }
    };
}