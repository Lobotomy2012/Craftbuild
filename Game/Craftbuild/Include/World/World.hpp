#pragma once

#include <core.hpp>
#include <World/enum.hpp>
#include <World/struct.hpp>
#include <World/variable.hpp>

namespace Craftbuild {
    class Frustum {
    private:
        enum Plane {
            LEFT = 0,
            RIGHT,
            BOTTOM,
            TOP,
            NEAR,
            FAR,
            COUNT
        };

        glm::vec4 planes[COUNT];

    public:
        void update(const glm::mat4& view_proj) {
            glm::mat4 mat = glm::transpose(view_proj);

            planes[LEFT] = mat[3] + mat[0];
            planes[RIGHT] = mat[3] - mat[0];
            planes[BOTTOM] = mat[3] + mat[1];
            planes[TOP] = mat[3] - mat[1];
            planes[NEAR] = mat[3] + mat[2];
            planes[FAR] = mat[3] - mat[2];

            for (int i = 0; i < COUNT; i++) {
                float length = glm::length(glm::vec3(planes[i]));
                planes[i] /= length;
            }
        }

        bool is_box_visible(const glm::vec3& min, const glm::vec3& max) const {
            for (int i = 0; i < COUNT; i++) {
                glm::vec3 positive = min;
                if (planes[i].x > 0) positive.x = max.x;
                if (planes[i].y > 0) positive.y = max.y;
                if (planes[i].z > 0) positive.z = max.z;

                float distance = planes[i].x * positive.x +
                    planes[i].y * positive.y +
                    planes[i].z * positive.z +
                    planes[i].w;

                if (distance < 0) {
                    return false;
                }
            }
            return true;
        }
    };

    class World {
    private:
        class PerlinNoise {
            static double fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }
            static double lerp(double t, double a, double b) { return a + t * (b - a); }
            static double grad(int hash, double x, double y, double z) {
                int h = hash & 15;
                double u = h < 8 ? x : y;
                double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
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

        double get_height(int x, int z) {
            double cont = continentalness.octave_noise(x * 0.001, z * 0.001, 0, 5, 0.5, 2.0) * 2.0 - 1.0;
            double ero = erosion.octave_noise(x * 0.002, z * 0.002, 0, 4, 0.6, 2.0) * 2.0 - 1.0;
            double pv = peaks_valleys.octave_noise(x * 0.003, z * 0.003, 0, 3, 0.7, 2.0) * 2.0 - 1.0;
            double w = weirdness.octave_noise(x * 0.004, z * 0.004, 0, 4, 0.5, 2.0) * 2.0 - 1.0;

            double base_height = 64.0 + cont * 80.0;
            double mountain = std::max(0.0, pv * 1.5 + w * 0.5) * 120.0;
            double eroded = ero * 40.0;

            return std::clamp(base_height + mountain - eroded, 0.0, 320.0);
        }

        BiomeType get_biome(int x, int z) {
            double temp = temperature.octave_noise(x * 0.0005, z * 0.0005, 0, 4, 0.5) * 2.0 - 1.0;
            double humid = humidity.octave_noise(x * 0.0007, z * 0.0007, 0, 4, 0.5) * 2.0 - 1.0;
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

            double forest = tree_noise.octave_noise(x * 0.002, z * 0.002, 0, 3, 0.6);
            if (forest > 0.4 && humid > 0.2) return BiomeType::FOREST;

            return BiomeType::PLAINS;
        }

        bool should_generate_cave(int x, int y, int z) {
            if (y < 10 || y > 240) return false;

            // Cheese caves (big round holes)
            double cheese = cave_noise.octave_noise(x * 0.03, y * 0.03, z * 0.03, 4, 0.5);
            // Noodle caves (thin tunnels)
            double noodle = cave_noise.octave_noise(x * 0.08, y * 0.08, z * 0.08, 3, 0.6);
            // Spaghetti caves (winding tunnels)
            double spaghetti = cave_noise.octave_noise(x * 0.05, y * 0.05, z * 0.05, 5, 0.4);

            double density = 0.65 - (y / 256.0) * 0.3;
            return (cheese > density * 1.2) || (noodle > density * 0.9) || (spaghetti > density * 1.0);
        }

        void generate_terrain(Chunk& chunk) {
            for (int lx = 0; lx < 16; ++lx) {
                for (int lz = 0; lz < 16; ++lz) {
                    int wx = chunk.x * 16 + lx;
                    int wz = chunk.z * 16 + lz;
                    double height = get_height(wx, wz);
                    BiomeType biome = get_biome(wx, wz);

                    for (int ly = 0; ly < 383; ++ly) {
                        if (ly == 0) {
                            chunk.blocks[lx][ly][lz] = BlockType::BEDROCK;
                        }
                        else if (ly < height - 4) {
                            chunk.blocks[lx][ly][lz] = BlockType::STONE;
                        }
                        else if (ly < height - 1) {
                            chunk.blocks[lx][ly][lz] = BlockType::DIRT;
                        }
                        else if (ly < height) {
                            switch (biome) {
                            case BiomeType::DESERT: case BiomeType::BEACH:
                                chunk.blocks[lx][ly][lz] = BlockType::SAND;
                                break;
                            case BiomeType::OCEAN:
                                chunk.blocks[lx][ly][lz] = (ly < 62) ? BlockType::SAND : BlockType::GRAVEL;
                                break;
                            default:
                                chunk.blocks[lx][ly][lz] = BlockType::GRASS;
                            }
                        }
                        else if (ly <= 62) {
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
            for (int lx = 0; lx < 16; ++lx) {
                for (int ly = 0; ly < 383; ++ly) {
                    for (int lz = 0; lz < 16; ++lz) {
                        int wx = chunk.x * 16 + lx;
                        int wz = chunk.z * 16 + lz;
                        BlockType& block = chunk.blocks[lx][ly][lz];
                        if (block != BlockType::AIR && block != BlockType::WATER && block != BlockType::BEDROCK) {
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
                        if (chunk.blocks[lx][ly][lz] == BlockType::GRASS ||
                            chunk.blocks[lx][ly][lz] == BlockType::DIRT ||
                            chunk.blocks[lx][ly][lz] == BlockType::SAND) {
                            ground_y = ly;
                            break;
                        }
                    }
                    if (ground_y < 4 || ground_y > 200) continue;

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
                                if (px < 0 || px >= 16 || pz < 0 || pz >= 16 || py < 0 || py >= 383) continue;

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
                        if (b == BlockType::GRASS || b == BlockType::DIRT || b == BlockType::SAND) {
                            ground_y = ly;
                            break;
                        }
                    }
                    if (ground_y < 0 || ground_y >= 382) continue;

                    if ((biome == BiomeType::TAIGA && rand(rng) < 0.7) ||
                        (biome == BiomeType::MOUNTAINS && ground_y > 140 && rand(rng) < 0.9)) {
                        if (chunk.blocks[lx][ground_y + 1][lz] == BlockType::AIR) {
                            chunk.blocks[lx][ground_y][lz] = BlockType::SNOW;
                        }
                    }

                    if (chunk.blocks[lx][ground_y + 1][lz] == BlockType::AIR) {
                        if (biome == BiomeType::PLAINS || biome == BiomeType::FOREST) {
                            if (rand(rng) < 0.25) {
                                chunk.blocks[lx][ground_y + 1][lz] = BlockType::GRASS_PLANT;
                            }
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
            tree_noise(seed + 7) {
        }

        void generate_chunk(Chunk& chunk) {
            generate_terrain(chunk);
            BiomeType biome = get_biome(chunk.x * CHUNK_SIZE + (CHUNK_SIZE / 2), chunk.z * CHUNK_SIZE + (CHUNK_SIZE / 2));
            generate_caves(chunk);
            generate_trees(chunk);
            decorate_surface(chunk, biome);
        }

        void set_seed(int new_seed) {
            seed = new_seed;
            continentalness = PerlinNoise(seed + 0);
            erosion = PerlinNoise(seed + 1);
            peaks_valleys = PerlinNoise(seed + 2);
            weirdness = PerlinNoise(seed + 3);
            temperature = PerlinNoise(seed + 4);
            humidity = PerlinNoise(seed + 5);
            cave_noise = PerlinNoise(seed + 6);
            tree_noise = PerlinNoise(seed + 7);
        }
    };
}