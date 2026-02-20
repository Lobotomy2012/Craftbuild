#pragma once

#include <Core/core.hpp>
#include <Core/Registry.hpp>
#include <World/Enum/NoiseParameter.hpp>

namespace Craftbuild {
	class Noise {
        static int64_t create_key(NoiseParameter noise_parameter) {
            return make_key((int)Registry::NOISE, (int)noise_parameter);
        }
    public:
        inline static int64_t TEMPERATURE = create_key(NoiseParameter::TEMPERATURE);
        inline static int64_t VEGETATION = create_key(NoiseParameter::VEGETATION);
        inline static int64_t CONTINENTALNESS = create_key(NoiseParameter::CONTINENTALNESS);
        inline static int64_t EROSION = create_key(NoiseParameter::EROSION);
        inline static int64_t TEMPERATURE_LARGE = create_key(NoiseParameter::TEMPERATURE_LARGE);
        inline static int64_t VEGETATION_LARGE = create_key(NoiseParameter::VEGETATION_LARGE);
        inline static int64_t CONTINENTALNESS_LARGE = create_key(NoiseParameter::CONTINENTALNESS_LARGE);
        inline static int64_t EROSION_LARGE = create_key(NoiseParameter::EROSION_LARGE);
        inline static int64_t RIDGE = create_key(NoiseParameter::RIDGE);
        inline static int64_t SHIFT = create_key(NoiseParameter::SHIFT);
        inline static int64_t AQUIFER_BARRIER = create_key(NoiseParameter::AQUIFER_BARRIER);
        inline static int64_t AQUIFER_FLUID_LEVEL_FLOODEDNESS = create_key(NoiseParameter::AQUIFER_FLUID_LEVEL_FLOODEDNESS);
        inline static int64_t AQUIFER_LAVA = create_key(NoiseParameter::AQUIFER_LAVA);
        inline static int64_t AQUIFER_FLUID_LEVEL_SPREAD = create_key(NoiseParameter::AQUIFER_FLUID_LEVEL_SPREAD);
        inline static int64_t PILLAR = create_key(NoiseParameter::PILLAR);
        inline static int64_t PILLAR_RARENESS = create_key(NoiseParameter::PILLAR_RARENESS);
        inline static int64_t PILLAR_THICKNESS = create_key(NoiseParameter::PILLAR_THICKNESS);
        inline static int64_t SPAGHETTI_2D = create_key(NoiseParameter::SPAGHETTI_2D);
        inline static int64_t SPAGHETTI_2D_ELEVATION = create_key(NoiseParameter::SPAGHETTI_2D_ELEVATION);
        inline static int64_t SPAGHETTI_2D_MODULATOR = create_key(NoiseParameter::SPAGHETTI_2D_MODULATOR);
        inline static int64_t SPAGHETTI_2D_THICKNESS = create_key(NoiseParameter::SPAGHETTI_2D_THICKNESS);
        inline static int64_t SPAGHETTI_3D_1 = create_key(NoiseParameter::SPAGHETTI_3D_1);
        inline static int64_t SPAGHETTI_3D_2 = create_key(NoiseParameter::SPAGHETTI_3D_2);
        inline static int64_t SPAGHETTI_3D_RARITY = create_key(NoiseParameter::SPAGHETTI_3D_RARITY);
        inline static int64_t SPAGHETTI_3D_THICKNESS = create_key(NoiseParameter::SPAGHETTI_3D_THICKNESS);
        inline static int64_t SPAGHETTI_ROUGHNESS = create_key(NoiseParameter::SPAGHETTI_ROUGHNESS);
        inline static int64_t SPAGHETTI_ROUGHNESS_MODULATOR = create_key(NoiseParameter::SPAGHETTI_ROUGHNESS_MODULATOR);
        inline static int64_t CAVE_ENTRANCE = create_key(NoiseParameter::CAVE_ENTRANCE);
        inline static int64_t CAVE_LAYER = create_key(NoiseParameter::CAVE_LAYER);
        inline static int64_t CAVE_CHEESE = create_key(NoiseParameter::CAVE_CHEESE);
        inline static int64_t ORE_VEININESS = create_key(NoiseParameter::ORE_VEININESS);
        inline static int64_t ORE_VEIN_A = create_key(NoiseParameter::ORE_VEIN_A);
        inline static int64_t ORE_VEIN_B = create_key(NoiseParameter::ORE_VEIN_B);
        inline static int64_t ORE_GAP = create_key(NoiseParameter::ORE_GAP);
        inline static int64_t NOODLE = create_key(NoiseParameter::NOODLE);
        inline static int64_t NOODLE_THICKNESS = create_key(NoiseParameter::NOODLE_THICKNESS);
        inline static int64_t NOODLE_RIDGE_A = create_key(NoiseParameter::NOODLE_RIDGE_A);
        inline static int64_t NOODLE_RIDGE_B = create_key(NoiseParameter::NOODLE_RIDGE_B);
        inline static int64_t JAGGED = create_key(NoiseParameter::JAGGED);
        inline static int64_t SURFACE = create_key(NoiseParameter::SURFACE);
        inline static int64_t SURFACE_SECONDARY = create_key(NoiseParameter::SURFACE_SECONDARY);
        inline static int64_t CLAY_BANDS_OFFSET = create_key(NoiseParameter::CLAY_BANDS_OFFSET);
        inline static int64_t BADLANDS_PILLAR = create_key(NoiseParameter::BADLANDS_PILLAR);
        inline static int64_t BADLANDS_PILLAR_ROOF = create_key(NoiseParameter::BADLANDS_PILLAR_ROOF);
        inline static int64_t BADLANDS_SURFACE = create_key(NoiseParameter::BADLANDS_SURFACE);
        inline static int64_t ICEBERG_PILLAR = create_key(NoiseParameter::ICEBERG_PILLAR);
        inline static int64_t ICEBERG_PILLAR_ROOF = create_key(NoiseParameter::ICEBERG_PILLAR_ROOF);
        inline static int64_t ICEBERG_SURFACE = create_key(NoiseParameter::ICEBERG_SURFACE);
        inline static int64_t SWAMP = create_key(NoiseParameter::SWAMP);
        inline static int64_t CALCITE = create_key(NoiseParameter::CALCITE);
        inline static int64_t GRAVEL = create_key(NoiseParameter::GRAVEL);
        inline static int64_t POWDER_SNOW = create_key(NoiseParameter::POWDER_SNOW);
        inline static int64_t PACKED_ICE = create_key(NoiseParameter::PACKED_ICE);
        inline static int64_t ICE = create_key(NoiseParameter::ICE);
        inline static int64_t SOUL_SAND_LAYER = create_key(NoiseParameter::SOUL_SAND_LAYER);
        inline static int64_t GRAVEL_LAYER = create_key(NoiseParameter::GRAVEL_LAYER);
        inline static int64_t PATCH = create_key(NoiseParameter::PATCH);
        inline static int64_t NETHERRACK = create_key(NoiseParameter::NETHERRACK);
        inline static int64_t NETHER_WART = create_key(NoiseParameter::NETHER_WART);
        inline static int64_t NETHER_STATE_SELECTOR = create_key(NoiseParameter::NETHER_STATE_SELECTOR);
	};
}