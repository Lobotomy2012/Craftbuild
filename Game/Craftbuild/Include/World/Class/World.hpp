#pragma once

#include <core.hpp>
#include <World/enum.hpp>
#include <World/variable.hpp>
#include <World/Struct/Block.hpp>
#include <World/Struct/Chunk.hpp>
#include <World/Class/Frustum.hpp>

namespace Craftbuild {
    class World {
        class PerlinNoise {
            static double fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }
            static double lerp(double t, double a, double b) { return a + t * (b - a); }
            static double grad(int hash, double x, double y, double z) {
                int h = hash & 15;
                double u = h < 8 ? x : y;
                double v = h < 4 ? y : (h == 12 or h == 14 ? x : z);
                return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
            }

        public:
            PerlinNoise(int seed = 0) {
                permutation.resize(WORLD_HEIGHT * 2);
                std::vector<int> p(WORLD_HEIGHT);
                for (int i = 0; i < WORLD_HEIGHT; ++i) p[i] = i;
                std::mt19937 engine(seed);
                std::shuffle(p.begin(), p.end(), engine);
                for (int i = 0; i < WORLD_HEIGHT; ++i) {
                    permutation[i] = permutation[i + WORLD_HEIGHT] = p[i];
                }
            }
            double noise(double x, double y, double z) const {
                int X = static_cast<int>(floor(x)) & (WORLD_HEIGHT - 1);
                int Y = static_cast<int>(floor(y)) & (WORLD_HEIGHT - 1);
                int Z = static_cast<int>(floor(z)) & (WORLD_HEIGHT - 1);
                x -= floor(x); y -= floor(y); z -= floor(z);
                double u = fade(x), v = fade(y), w = fade(z);
                int A = permutation[X] + Y, AA = permutation[A] + Z, AB = permutation[A + 1] + Z;
                int B = permutation[X + 1] + Y, BA = permutation[B] + Z, BB = permutation[B + 1] + Z;
                return lerp(w, lerp(v, lerp(u, grad(permutation[AA], x, y, z),
                    grad(permutation[BA], x - 1, y, z)),
                    lerp(u, grad(permutation[AB], x, y - 1, z),
                        grad(permutation[BB], x - 1, y - 1, z))),
                    lerp(v, lerp(u, grad(permutation[AA + 1], x, y, z - 1),
                        grad(permutation[BA + 1], x - 1, y, z - 1)),
                        lerp(u, grad(permutation[AB + 1], x, y - 1, z - 1),
                            grad(permutation[BB + 1], x - 1, y - 1, z - 1))));
            }
            double octave_noise(double x, double y, double z, int octaves, double persistence, double lacunarity = 2.0) const {
                double total = 0.0, amplitude = 1.0, frequency = 1.0, maxValue = 0.0;
                for (int i = 0; i < octaves; ++i) {
                    total += noise(x * frequency, y * frequency, z * frequency) * amplitude;
                    maxValue += amplitude;
                    amplitude *= persistence;
                    frequency *= lacunarity;
                }
                return total / maxValue;
            }

        private:
            std::vector<int> permutation;
        };

        PerlinNoise continentalness, erosion, peaks_valleys, weirdness, temperature, humidity, cave_noise, tree_noise;
        int seed;

        double compute_density(int x, int y, int z) {
            double cont = continentalness.octave_noise(x * 0.0008, 0, z * 0.0008, 5, 0.5) * 2.0 - 1.0;
            double ero = erosion.octave_noise(x * 0.0015, 0, z * 0.0015, 4, 0.6) * 2.0 - 1.0;
            double pv = peaks_valleys.octave_noise(x * 0.002, 0, z * 0.002, 4, 0.5) * 2.0 - 1.0;
            double weird = weirdness.octave_noise(x * 0.0012, 0, z * 0.0012, 3, 0.5) * 2.0 - 1.0;

            double terrain = cont * 1.2 - ero * 0.5 + pv * 0.8 + weird * 0.3;

            // Vertical gradient
            double vertical = (y - 80.0) / 80.0;

            // Cave density
            double cave = cave_noise.octave_noise(x * 0.03, y * 0.03, z * 0.03, 4, 0.5);
            return terrain - vertical - cave * 0.7;
        }

        BiomeType get_biome(int x, int z) {
            double cont = continentalness.octave_noise(x * 0.0008, 0, z * 0.0008, 5, 0.5) * 2.0 - 1.0;
            double temp = temperature.octave_noise(x * 0.0005, 0, z * 0.0005, 4, 0.5) * 2.0 - 1.0;
            double humid = humidity.octave_noise(x * 0.0007, 0, z * 0.0007, 4, 0.5) * 2.0 - 1.0;

            if (cont < -0.4) return BiomeType::OCEAN;
            if (cont < -0.2) return BiomeType::BEACH;

            if (temp < -0.5) return BiomeType::TAIGA;

            if (temp > 0.6) {
                if (humid < -0.2) return BiomeType::DESERT;
                if (humid > 0.5) return BiomeType::JUNGLE;
                return BiomeType::SAVANNA;
            }

            if (humid > 0.6) return BiomeType::SWAMP;

            return BiomeType::PLAINS;
        }

        void generate_terrain(Chunk& chunk) {
            for (int lx = 0; lx < CHUNK_SIZE; ++lx) {
                for (int lz = 0; lz < CHUNK_SIZE; ++lz) {
                    int wx = chunk.x * CHUNK_SIZE + lx;
                    int wz = chunk.z * CHUNK_SIZE + lz;

                    for (int ly = 0; ly < WORLD_HEIGHT; ++ly) {
                        int wy = ly;

                        if (wy == 0) {
                            chunk.blocks[lx][ly][lz] = BlockType::BEDROCK;
                            continue;
                        }
                        else if (wy < 5) {
                            chunk.blocks[lx][ly][lz] = BlockType::STONE;
                            continue;
                        }

                        double density = compute_density(wx, wy, wz);

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

        void generate_trees(Chunk& chunk) {
            std::mt19937 rng(seed + chunk.x * 73856093 + chunk.z * 19349663);
            std::uniform_int_distribution<> height_dist(5, 12);
            std::uniform_real_distribution<> rand(0.0, 1.0);

            for (int lx = 0; lx < CHUNK_SIZE; ++lx) {
                for (int lz = 0; lz < CHUNK_SIZE; ++lz) {
                    int wx = chunk.x * CHUNK_SIZE + lx;
                    int wz = chunk.z * CHUNK_SIZE + lz;
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
                    for (int ly = WORLD_HEIGHT; ly >= 0; --ly) {
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
                        if (ty >= WORLD_HEIGHT) break;
                        chunk.blocks[lx][ty][lz] = BlockType::WOOD;
                    }

                    int leaf_y = ground_y + height;
                    int radius = (biome == BiomeType::JUNGLE) ? 5 : 4;
                    for (int dx = -radius; dx <= radius; ++dx) {
                        for (int dz = -radius; dz <= radius; ++dz) {
                            for (int dy = -radius / 2; dy <= radius; ++dy) {
                                int px = lx + dx, pz = lz + dz, py = leaf_y + dy;
                                if (px < 0 or px >= CHUNK_SIZE or pz < 0 or pz >= CHUNK_SIZE or py < 0 or py >= WORLD_HEIGHT) continue;

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

        void decorate_surface(Chunk& chunk, BiomeType biome) {
            std::mt19937 rng(seed + chunk.x * 10007 + chunk.z * 10009);
            std::uniform_real_distribution<> rand(0.0, 1.0);

            for (int lx = 0; lx < 16; ++lx) {
                for (int lz = 0; lz < 16; ++lz) {
                    int ground_y = -1;
                    for (int ly = 382; ly >= 0; --ly) {
                        BlockType b = chunk.blocks[lx][ly][lz];
                        if (b == BlockType::GRASS or b == BlockType::DIRT or b == BlockType::SAND) {
                            ground_y = ly;
                            break;
                        }
                    }
                    if (ground_y < 0 or ground_y >= 382) continue;

                    if ((biome == BiomeType::TAIGA and rand(rng) < 0.7) or
                        (biome == BiomeType::MOUNTAINS and ground_y > 140 and rand(rng) < 0.9)) {
                        if (chunk.blocks[lx][ground_y + 1][lz] == BlockType::AIR) {
                            chunk.blocks[lx][ground_y][lz] = BlockType::SNOW;
                        }
                    }

                    if (chunk.blocks[lx][ground_y + 1][lz] == BlockType::AIR) {
                        if (biome == BiomeType::PLAINS or biome == BiomeType::FOREST) {
                            if (rand(rng) < 0.25) {
                                chunk.blocks[lx][ground_y + 1][lz] = BlockType::GRASS_PLANT;
                            }
                        }
                    }
                }
            }
        }

        void build_surface(Chunk& chunk) {
            for (int lx = 0; lx < CHUNK_SIZE; ++lx) {
                for (int lz = 0; lz < CHUNK_SIZE; ++lz) {
                    int depth = 0;

                    for (int ly = WORLD_HEIGHT - 1; ly >= 0; --ly) {
                        BlockType& b = chunk.blocks[lx][ly][lz];

                        if (b == BlockType::AIR) {
                            depth = 0;
                        }
                        else if (b == BlockType::STONE) {
                            if (depth == 0 && ly > 62) {
                                b = BlockType::GRASS;
                            }
                            else if (depth < 4) {
                                b = BlockType::DIRT;
                            }
                            depth++;
                        }
                    }
                }
            }
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
            build_surface(chunk);
            generate_trees(chunk);
        }

        void set_seed(int seed) {
            this->seed = seed;
            continentalness = PerlinNoise(seed);
            erosion = PerlinNoise(seed + 1);
            peaks_valleys = PerlinNoise(seed + 2);
            weirdness = PerlinNoise(seed + 3);
            temperature = PerlinNoise(seed + 4);
            humidity = PerlinNoise(seed + 5);
            cave_noise = PerlinNoise(seed + 6);
            tree_noise = PerlinNoise(seed + 7);
        }

        int get_seed() const {
            return seed;
        }
    };
}