#pragma once

#include <Core/core.hpp>
#include <World/Enum/BlockType.hpp>

namespace Craftbuild {
	constexpr uint8_t CHUNK_SIZE = 16;
	constexpr uint16_t WORLD_HEIGHT = 384;
	constexpr int TEXTURE_AMOUNT = 13;

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