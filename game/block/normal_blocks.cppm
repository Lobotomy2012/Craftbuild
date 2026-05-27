module;

#include <includes.hpp>

export module game.block.normal_blocks;

import misc.str;
import misc.number;
import game.block;

export namespace craftbuild {
	class AirBlock : public Block1F {
	public:
		std::vector<std::pair<Str, uint64>> init_tags() override {
			return { { "transparent", true } };
		}
	};
	class DirtBlock : public Block1F {};
	class GrassBlock : public Block3F {};
	class StoneBlock : public Block1F {};
	class BedrockBlock : public Block1F {};
}