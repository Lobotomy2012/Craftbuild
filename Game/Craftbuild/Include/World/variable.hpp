#pragma once

#include <core.hpp>
#include <World/enum.hpp>

namespace Craftbuild {
	const uint8_t CHUNK_SIZE = 16;
	const uint16_t WORLD_HEIGHT = 384;
	const int TEXTURE_AMOUNT = 13;

	const std::vector<BlockType> block_types = {
		BlockType::GRASS,
		BlockType::DIRT,
		BlockType::STONE,
		BlockType::DIAMOND_BLOCK,
		BlockType::WATER,
		BlockType::SAND,
		BlockType::WOOD,
		BlockType::LEAVES,
		BlockType::BEDROCK,
		BlockType::GRAVEL,
		BlockType::SNOW,
		BlockType::GRASS_PLANT,
		BlockType::GLASS
	};
}