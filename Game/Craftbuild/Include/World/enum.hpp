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
}