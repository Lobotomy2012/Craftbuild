#pragma once

#include <core.hpp>

namespace Craftbuild {
    enum class BlockType : uint8_t {
        GRASS = 0,
        DIRT,
        STONE,
        DIAMOND_BLOCK,
        WATER,
        SAND,
        WOOD,
        LEAVES,
        BEDROCK,
        GRAVEL,
        SNOW,
        GRASS_PLANT,
        GLASS,
        AIR = 255
    };

    enum class BiomeType {
        PLAINS,
        MOUNTAINS,
        DESERT,
        FOREST,
        OCEAN,
        RIVER,
        BEACH,
        TAIGA,
        SWAMP,
        JUNGLE,
        SAVANNA
    };

    enum class LODLevel {
        LEVEL_0 = 0,
        LEVEL_1,
        LEVEL_2,
        LEVEL_3,
    };
}