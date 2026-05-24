module;

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include <includes.hpp>
#include <mutex>
#include <shared_mutex>
#include <algorithm>
#include <memory>

export module game.world.chunk;

import misc.ptr;
import misc.types;
import game.pos;
import game.block;
import game.logger;
import game.world.biome;
import game.world.terrain;

using namespace godot;

export namespace craftbuild {
    struct MeshData {
        std::vector<Pos<real>> vertices;
        std::vector<Pos<real>> normals;
        std::vector<int32> indices;
        std::vector<Vector2> uvs;
        std::vector<Vector2> uvs_layer;
        std::vector<Pos<real>> collision_faces;
    };

    struct Chunk {
        inline static constexpr uint8 SIZE_X = 16;
        inline static constexpr uint8 SIZE_Y = 255;
        inline static constexpr uint8 SIZE_Z = 16;

        std::unordered_map<uint8, uint32> block_ids;
        std::unordered_map<uint8, std::pair<uint32, uint16>> tag_ids;
        std::unordered_map<Pos<uint8>, BlockStorageFull, PosHash<uint8>> complex_blocks;
        BlockStorage blocks[SIZE_X][SIZE_Y][SIZE_Z] = {};

        MeshInstance3D* mesh_instance = nullptr;
        Vector3i chunk_pos;
        TrapezoidHeight height_provider{ VerticalAnchor::absolute(18), VerticalAnchor::absolute(38), 8 };

        std::atomic<bool> generated = false;
        std::atomic<bool> dirty = true;
        std::atomic<bool> collision_built = false;

        std::atomic<bool> mesh_ready{ false };
        ptr<MeshData> pending_mesh_data = nullptr;
        mutable std::mutex mesh_mutex;
        mutable std::shared_mutex data_mutex;

        static uint32 column_seed(int32 seed, int32 x, int32 z) {
            uint32 h = static_cast<uint32>(seed);
            h ^= static_cast<uint32>(x) + 0x9e3779b9u + (h << 6) + (h >> 2);
            h ^= static_cast<uint32>(z) + 0x85ebca6bu + (h << 6) + (h >> 2);
            h ^= h >> 16;
            h *= 0x7feb352du;
            h ^= h >> 15;
            h *= 0x846ca68bu;
            h ^= h >> 16;
            return h;
        }

        ~Chunk() {
            std::lock_guard lock(mesh_mutex);
            if (pending_mesh_data) {
                pending_mesh_data.clear();
            }
        }

        none set_block(const Pos<uint8>& pos, const std::string& block) {
			set_block(pos, BlockRegistry::get_id(block));
        }
        none set_block(const Pos<uint8>& pos, uint32 block_id) {
            std::unique_lock lock(data_mutex);
            if (block_ids.size() >= 256) {
                complex_blocks.emplace(pos, BlockStorageFull(block_id, 0, 0));
                return;
            }

            auto it = std::find_if(block_ids.begin(), block_ids.end(), [block_id](const auto& pair) {
                return pair.second == block_id;
            });
            if (it != block_ids.end()) {
                blocks[pos.x][pos.y][pos.z].block_id = it->first;
                return;
            }

            blocks[pos.x][pos.y][pos.z].block_id = block_ids.size();
            block_ids.emplace(block_ids.size(), block_id);
        }

        none tag_block(const Pos<uint8>& pos, const std::string& tag, uint16 tag_data = 0) {
            tag_block(pos, TagRegistry::get_id(tag), tag_data);
        }
        none tag_block(const Pos<uint8>& pos, uint32 tag_id, uint16 tag_data = 0) {
            std::unique_lock lock(data_mutex);
            if (tag_ids.size() >= 256) {
				complex_blocks[pos].tag = tag_id;
                return;
            }

            auto it = std::find_if(tag_ids.begin(), tag_ids.end(), [tag_id, tag_data](const auto& pair) {
                return pair.second == std::make_pair(tag_id, tag_data);
            });
            if (it != tag_ids.end()) {
                blocks[pos.x][pos.y][pos.z].tag = it->first;
                return;
            }

            blocks[pos.x][pos.y][pos.z].tag = tag_ids.size();
            tag_ids.emplace(tag_ids.size(), std::make_pair(tag_id, (uint16)0));
        }

        bool has_tag(const Pos<uint8>& pos, const std::string& tag) const {
			return has_tag(pos, TagRegistry::get_id(tag));
        }
        bool has_tag(const Pos<uint8>& pos, uint32 tag_id) const {
            std::shared_lock lock(data_mutex);
            if (pos.x >= SIZE_X or pos.y >= SIZE_Y or pos.z >= SIZE_Z) return false;

            if (tag_ids.size() >= 256) {
                return complex_blocks.at(pos).tag == tag_id;
            }

            auto& block_tag = blocks[pos.x][pos.y][pos.z].tag;
			return tag_ids.find(block_tag) != tag_ids.end() and tag_ids.at(block_tag).first == tag_id;
        }

        template <bool lock = true>
        uint32 get_block(const Pos<uint8>& pos) const {}
        template <bool lock = true>
		std::pair<uint32, uint16> get_tag(const Pos<uint8>& pos) const {}

        template<>
        uint32 get_block<true>(const Pos<uint8>& pos) const {
            std::shared_lock lock(data_mutex);
            if (pos.x >= SIZE_X or pos.y >= SIZE_Y or pos.z >= SIZE_Z) return 0;
            auto it = complex_blocks.find(pos);
            if (it != complex_blocks.end()) {
                return it->second.block_id;
            }
            return block_ids.find(blocks[pos.x][pos.y][pos.z].block_id) != block_ids.end() ? block_ids.at(blocks[pos.x][pos.y][pos.z].block_id) : 0;
        }
        template<>
        std::pair<uint32, uint16> get_tag<true>(const Pos<uint8>& pos) const {
            std::shared_lock lock(data_mutex);
            if (pos.x >= SIZE_X or pos.y >= SIZE_Y or pos.z >= SIZE_Z) return std::make_pair(0, 0);
            auto it = complex_blocks.find(pos);
            if (it != complex_blocks.end()) {
                return std::make_pair(it->second.tag, it->second.tag_data);
            }
            return tag_ids.find(blocks[pos.x][pos.y][pos.z].tag) != tag_ids.end() ? tag_ids.at(blocks[pos.x][pos.y][pos.z].tag) : std::make_pair(0U, (uint16)0);
        }

        template<>
        uint32 get_block<false>(const Pos<uint8>& pos) const {
            if (pos.x >= SIZE_X or pos.y >= SIZE_Y or pos.z >= SIZE_Z) return 0;
            auto it = complex_blocks.find(pos);
            if (it != complex_blocks.end()) {
                return it->second.block_id;
            }
            return block_ids.find(blocks[pos.x][pos.y][pos.z].block_id) != block_ids.end() ? block_ids.at(blocks[pos.x][pos.y][pos.z].block_id) : 0;
        }
        template<>
        std::pair<uint32, uint16> get_tag<false>(const Pos<uint8>& pos) const {
            if (pos.x >= SIZE_X or pos.y >= SIZE_Y or pos.z >= SIZE_Z) return std::make_pair(0, 0);
            auto it = complex_blocks.find(pos);
            if (it != complex_blocks.end()) {
                return std::make_pair(it->second.tag, it->second.tag_data);
            }
            return tag_ids.find(blocks[pos.x][pos.y][pos.z].tag) != tag_ids.end() ? tag_ids.at(blocks[pos.x][pos.y][pos.z].tag) : std::make_pair(0U, (uint16)0);
        }

        none generate_terrain(int32 seed, ref<FastNoiseLite> noise) {
            const uint32 AIR   = BlockRegistry::get_id("Air");
            const uint32 GRASS = BlockRegistry::get_id("Grass Block");
            const uint32 DIRT  = BlockRegistry::get_id("Dirt");
            const uint32 STONE = BlockRegistry::get_id("Stone");
            const uint32 BEDROCK = BlockRegistry::get_id("Bedrock");

            auto new_blocks = std::make_unique<BlockStorage[][SIZE_Y][SIZE_Z]>(SIZE_X);
            std::unordered_map<uint8, uint32> new_block_ids;
            std::unordered_map<uint8, std::pair<uint32, uint16>> new_tag_ids;
            std::unordered_map<Pos<uint8>, BlockStorageFull, PosHash<uint8>> new_complex_blocks;

            auto add_block_unlocked = [&](const Pos<uint8>& pos, uint32 block_id) {
                if (new_block_ids.size() >= 256) {
                    new_complex_blocks.emplace(pos, BlockStorageFull(block_id, 0, 0));
                    return;
                }

                auto it = std::find_if(new_block_ids.begin(), new_block_ids.end(), [block_id](const auto& pair) {
                    return pair.second == block_id;
                });
                if (it != new_block_ids.end()) {
                    new_blocks[pos.x][pos.y][pos.z].block_id = it->first;
                    return;
                }

                const uint8 local_id = static_cast<uint8>(new_block_ids.size());
                new_blocks[pos.x][pos.y][pos.z].block_id = local_id;
                new_block_ids.emplace(local_id, block_id);
            };

            size biome_count = BiomeRegistry::registry.size();
            for (uint8 x = 0; x < SIZE_X; ++x) {
                for (uint8 z = 0; z < SIZE_Z; ++z) {
                    int32 global_x = chunk_pos.x * SIZE_X + x;
                    int32 global_z = chunk_pos.z * SIZE_Z + z;

                    Biome current_biome = { 0.01f, 40.0f, 0.4f, 4.0f, 60.0f, 0.5f };
                    if (biome_count > 0) {
                        float32 biome_noise_val = noise->get_noise_2d(
                            static_cast<real_t>(global_x + 10000) * 0.002f,
                            static_cast<real_t>(global_z + 10000) * 0.002f
                        );
                        float32 normalized = (biome_noise_val + 1.0f) * 0.5f;
                        size biome_idx = std::clamp(static_cast<size>(normalized * biome_count), static_cast<size>(0), biome_count - 1);
                        current_biome = BiomeRegistry::get_biome(biome_idx);
                    }

                    float32 base_noise = noise->get_noise_2d(static_cast<real_t>(global_x) * current_biome.base_noise, static_cast<real_t>(global_z) * current_biome.base_noise);
                    float32 base_elevation = ((base_noise + 1.0f) * 0.5f) * current_biome.base_height;
                    float32 terrain_base_y = current_biome.min_height + base_elevation;

                    int solid_depth = -1;

                    for (int y = SIZE_Y - 1; y >= 0; --y) {
                        if (y == 0) {
                            add_block_unlocked({ x, (uint8)y, z }, BEDROCK);
                            continue;
                        }

                        float32 noise_3d = noise->get_noise_3d(
                            static_cast<real_t>(global_x) * 0.2f,
                            static_cast<real_t>(y) * 0.3f,
                            static_cast<real_t>(global_z) * 0.2f
                        );

                        float32 density = terrain_base_y - static_cast<float32>(y) + (noise_3d * 25.0f);
                        uint32 block_id = AIR;

                        if (density > 0.0f) {
                            if (solid_depth == -1) {
                                block_id = GRASS;
                                solid_depth = 1;
                            }
                            else if (solid_depth < 4) {
                                block_id = DIRT;
                                solid_depth++;
                            }
                            else block_id = STONE;
                        }
                        else solid_depth = -1;

                        add_block_unlocked({ x, (uint8)y, z }, block_id);
                    }
                }
            }

            {
                std::unique_lock lock(data_mutex);
                std::memcpy(blocks, new_blocks.get(), sizeof(blocks));
                block_ids = std::move(new_block_ids);
                tag_ids = std::move(new_tag_ids);
                complex_blocks = std::move(new_complex_blocks);
            }

            generated.store(true, std::memory_order_release);
            dirty.store(true, std::memory_order_release);
        }
    };
}
