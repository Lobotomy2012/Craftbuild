module;

#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/variant/node_path.hpp>

#include <includes.hpp>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <thread>
#include <shared_mutex>
#include <chrono>
#include <cstring>
#include <random>

module game.main;

import game.player;

namespace craftbuild {
    none Main::init() {
        start_log_thread();

        TagRegistry::register_tag("face");
        TagRegistry::register_tag("transparent");

        BlockRegistry::register_block<AirBlock>("Air", "");
        BlockRegistry::register_block<GrassBlock>("Grass Block", "grass_block.png");
        BlockRegistry::register_block<DirtBlock>("Dirt", "dirt.png");
        BlockRegistry::register_block<StoneBlock>("Stone", "stone.png");
        BlockRegistry::register_block<BedrockBlock>("Bedrock", "bedrock.png");

        Biome plains;
        plains.base_height = 5.0f;
        plains.base_noise = 0.1f;
        plains.detail_height = 4.0f;
        plains.detail_noise = 0.2f;
        plains.min_height = 40;

        Biome normal;
        normal.base_height = 60.0f;
        normal.base_noise = 0.05f;
        normal.detail_height = 8.0f;
        normal.detail_noise = 0.5f;
        normal.min_height = 40;

        Biome mountains;
        mountains.base_height = 140.0f;
        mountains.base_noise = 0.02f;
        mountains.detail_height = 35.0f;
        mountains.detail_noise = 0.15f;
        mountains.min_height = 40;

        BiomeRegistry::register_biome("Plains", plains);
        BiomeRegistry::register_biome("Normal", normal);
        BiomeRegistry::register_biome("Mountains", mountains);

        if (not load_userdata()) log<LogType::WARNING>("Userdata file not found.");
        if (not load_world(format{} << "user://game/saves/" << world_name << "/overworld.cbsave")) {
            log<LogType::WARNING>("Save file not found, starting new world.");
            if (world_seed.load(std::memory_order_acquire) == 0) {
                std::mt19937 generator;
                std::uniform_int_distribution<int32> distribution;
                world_seed.store(distribution(generator), std::memory_order_release);
            }
        }
        noise.instantiate();
        noise->set_noise_type(FastNoiseLite::TYPE_SIMPLEX);
        noise->set_frequency(0.0125f);
        noise->set_seed(world_seed.load(std::memory_order_acquire));

        AtlasTexture::build_texture_array();
        setup_voxel_material();

        if (not get_node_or_null(NodePath("Sun"))) add_child(memnew(Sun));
        if (not get_node_or_null(NodePath("Sky"))) add_child(memnew(CraftSky));

        player_ptr = get_node<Player>("Player");

        log<LogType::VERBOSE>("Assets loaded");

        world_ready.store(true, std::memory_order_release);
        start_redstone_thread();
        start_scheduler_thread();

        log<LogType::INFO>("Main initialized");
    }

    none Main::_ready() {
        init();
    }

    none Main::_process(float64 delta) {
        Player* player = static_cast<Player*>(player_ptr);
        if (not player) return;

		const auto player_pos = player->get_global_position();
        player_x.store(player_pos.x, std::memory_order_relaxed);
        player_y.store(player_pos.y, std::memory_order_relaxed);
        player_z.store(player_pos.z, std::memory_order_relaxed);

        if (not world_ready.load(std::memory_order_acquire)) return;

        std::vector<ptr<Chunk>> chunks_to_upload;
        {
            std::shared_lock lock(chunks_mutex);
            for (const auto& E : chunks) {
                if (E.second.value().mesh_ready.load(std::memory_order_acquire)) {
                    chunks_to_upload.push_back(E.second);
                }
            }
        }

        static const int max_updates = 4;
        int updates_this_frame = 0;

        for (auto& chunk_ptr : chunks_to_upload) {
            if (updates_this_frame >= max_updates) break;

            ptr<MeshData> data = nullptr;
            {
                std::lock_guard lock(chunk_ptr.value().mesh_mutex);
                if (chunk_ptr.value().pending_mesh_data) {
                    data = chunk_ptr.value().pending_mesh_data;
					chunk_ptr.value().pending_mesh_data.clear();
                    chunk_ptr.value().mesh_ready.store(false, std::memory_order_release);
                }
            }

            if (not data) continue;

            ref<ArrayMesh> mesh;
            mesh.instantiate();

            if (data.value().vertices.size() != 0) {
                Array arrays;
                arrays.resize(Mesh::ARRAY_MAX);

                PackedVector3Array vertices;
                vertices.resize(data.value().vertices.size());
                memcpy(vertices.ptrw(), data.value().vertices.data(), data.value().vertices.size() * sizeof(Pos<float32>));

                PackedVector3Array normals;
                normals.resize(data.value().normals.size());
                memcpy(normals.ptrw(), data.value().normals.data(), data.value().normals.size() * sizeof(Pos<float32>));

                PackedInt32Array indices;
                indices.resize(data.value().indices.size());
                memcpy(indices.ptrw(), data.value().indices.data(), data.value().indices.size() * sizeof(int32));

                PackedVector2Array uvs;
                uvs.resize(data.value().uvs.size());
                memcpy(uvs.ptrw(), data.value().uvs.data(), data.value().uvs.size() * sizeof(Vector2));

                PackedVector2Array uvs_layer;
                uvs_layer.resize(data.value().uvs_layer.size());
                memcpy(uvs_layer.ptrw(), data.value().uvs_layer.data(), data.value().uvs_layer.size() * sizeof(Vector2));

                PackedVector3Array collision_faces;
                collision_faces.resize(data.value().collision_faces.size());
                memcpy(collision_faces.ptrw(), data.value().collision_faces.data(), data.value().collision_faces.size() * sizeof(Pos<float32>));

                arrays[Mesh::ARRAY_VERTEX] = vertices;
                arrays[Mesh::ARRAY_NORMAL] = normals;
                arrays[Mesh::ARRAY_INDEX] = indices;
                arrays[Mesh::ARRAY_TEX_UV] = uvs;
                arrays[Mesh::ARRAY_TEX_UV2] = uvs_layer;

                mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
                update_chunk_mesh(chunk_ptr, mesh, collision_faces);
            }

            chunk_ptr.value().collision_built.store(false, std::memory_order_release);

            updates_this_frame++;
        }

        std::vector<Pos<int>> pending_unloads;

        if (not should_remove_chunks.load(std::memory_order_acquire)) return;

        {
            std::lock_guard lock(chunks_to_remove_mutex);
            if (chunks_to_remove.empty()) {
                should_remove_chunks.store(false, std::memory_order_release);
                return;
            }
            pending_unloads.swap(chunks_to_remove);
            should_remove_chunks.store(false, std::memory_order_release);
        }

        std::unique_lock lock(chunks_mutex);
        for (const auto& pos : pending_unloads) {
            if (not chunks.contains(pos)) continue;

            auto chunk_ptr = chunks[pos];
            if (chunk_ptr.value().mesh_instance) chunk_ptr.value().mesh_instance->queue_free();
            chunks.erase(pos);
        }
    }

    none Main::_notification(int p_what) {
        std::string file_name = format{} << "user://game/saves/" << world_name << "/overworld.cbsave";
        if (p_what == NOTIFICATION_WM_CLOSE_REQUEST) save_world(file_name); // Auto save
        else if (p_what == NOTIFICATION_APPLICATION_FOCUS_OUT) emit_signal("pause");
        else if (p_what == NOTIFICATION_EXIT_TREE) {
			running.store(false, std::memory_order_relaxed);

            if (log_thread.joinable()) log_thread.join();
            if (redstone_thread.joinable()) redstone_thread.join();
            if (scheduler_thread.joinable()) scheduler_thread.join();
            loop_cv.notify_all();

            save_world(file_name);
            save_userdata();
        }
    }

    none Main::_input(const ref<InputEvent>& event) {
        // ESC: Lock/Unlock mouse
        if (event->is_action_pressed("ui_cancel")) {
            if (not pausing.load(std::memory_order_relaxed)) emit_signal("pause");
            else emit_signal("resume");
        }
    }

    none Main::setup_voxel_material() {
        ref<ShaderMaterial> mat;
        mat.instantiate();

        ref<Shader> shader;
        shader.instantiate();
        shader->set_code(R"(
            shader_type spatial;
            render_mode cull_back, depth_draw_opaque, diffuse_burley;

            uniform sampler2DArray u_texture_array : source_color, filter_linear_mipmap;

            varying float v_layer;

            void vertex() {
                v_layer = UV2.x;
            }

            void fragment() {
                vec3 uvw = vec3(UV, v_layer);
                vec4 tex = texture(u_texture_array, uvw);

                if (tex.a < 0.1) {
                    discard;
                }

                ALBEDO = tex.rgb;
            }
        )");

        mat->set_shader(shader);
        mat->set_shader_parameter("u_texture_array", AtlasTexture::atlas_texture);

        world_material = mat;
    }

    none Main::start_log_thread() {
        if (log_thread.joinable()) return;

        auto worker = [this]() {
            ThreadRegistry::register_thread("Log Thread");
            log<LogType::INFO>("Log thread started");

            while (running.load(std::memory_order_relaxed)) {
                LogQueue::flush();
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            LogQueue::flush();
        };

        log_thread = std::thread(worker);
    }

    none Main::start_redstone_thread() {
        if (redstone_thread.joinable()) return;

        auto worker = [this]() {
            ThreadRegistry::register_thread("Redstone Thread");
            log<LogType::INFO>("Redstone thread started");

            while (running.load(std::memory_order_relaxed)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        };

        redstone_thread = std::thread(worker);
    }

    none Main::start_scheduler_thread() {
        scheduler_thread = std::thread([this]() {
            ThreadRegistry::register_thread("Scheduler Thread");

            auto last_unload_time = std::chrono::high_resolution_clock::now();
            while (running.load()) {
                submit_jobs();

                auto now = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::seconds>(now - last_unload_time).count() >= 5) {
                    unload_distant_chunks((int)(player_x.load() / Chunk::SIZE_X), (int)(player_z.load() / Chunk::SIZE_Z));
                    last_unload_time = now;
                }

                std::unique_lock<std::mutex> lock(loop_mutex);
                loop_cv.wait_for(lock, std::chrono::milliseconds(100));
            }
        });
    }

    none Main::submit_jobs() {
        const int px = (int)std::floor(player_x.load() / Chunk::SIZE_X);
        const int pz = (int)std::floor(player_z.load() / Chunk::SIZE_Z);
        static constexpr int max_jobs_per_tick = 32;
        int submitted = 0;

        for (int r = 0; r <= render_distance; ++r) {
            for (int x = -r; x <= r; ++x) {
                for (int z = -r; z <= r; ++z) {
                    if (std::abs(x) != r and std::abs(z) != r) continue;

                    Pos<int> chunk_pos{ px + x, 0, pz + z };
                    auto chunk = get_or_create_chunk(chunk_pos);

                    // Terrain
                    if (not chunk.value().generated.load(std::memory_order_acquire)) {
                        {
                            std::lock_guard lock(pending_jobs_mutex);
                            if (pending_terrain_jobs.contains(chunk_pos)) continue;
                            pending_terrain_jobs.insert(chunk_pos);
                        }

                        terrain_pool.enqueue([this, chunk, chunk_pos]() {
                            if (running.load()) {
                                if (not chunk.value().generated.load(std::memory_order_acquire)) {
                                    chunk.value().generate_terrain(world_seed.load(), noise);
                                    chunk.value().dirty.store(true, std::memory_order_release);

                                    Pos<int> offsets[4] = { {1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1} };
                                    for (auto& o : offsets) {
                                        auto n = get_chunk(chunk.value().chunk_pos.x + o.x, chunk.value().chunk_pos.z + o.z);
                                        if (n and n.value().generated.load(std::memory_order_acquire)) n.value().dirty.store(true);
                                    }
                                }
                            }

                            std::lock_guard lock(pending_jobs_mutex);
                            pending_terrain_jobs.erase(chunk_pos);
                        });
                    }

                    // Mesh
                    if (not chunk.value().dirty.load(std::memory_order_acquire) or chunk.value().mesh_ready.load(std::memory_order_acquire)) continue;
                    
                    {
                        std::lock_guard lock(pending_jobs_mutex);
                        if (pending_mesh_jobs.contains(chunk_pos)) continue;
                        pending_mesh_jobs.insert(chunk_pos);
                    }

                    mesh_pool.enqueue([this, chunk, chunk_pos]() {
                        if (running.load(std::memory_order_relaxed)) {
                            auto& _chunk = chunk.value();
                            if (_chunk.dirty.load() or _chunk.mesh_ready.load()) {
                                ptr<Chunk> neighbors[4] = {
                                    get_chunk(_chunk.chunk_pos.x + 1, _chunk.chunk_pos.z),
                                    get_chunk(_chunk.chunk_pos.x - 1, _chunk.chunk_pos.z),
                                    get_chunk(_chunk.chunk_pos.x, _chunk.chunk_pos.z + 1),
                                    get_chunk(_chunk.chunk_pos.x, _chunk.chunk_pos.z - 1)
                                };

                                _chunk.generate_mesh(neighbors);
                                _chunk.mesh_ready.store(true);
                                _chunk.dirty.store(false);
                            }
                        }

                        std::lock_guard lock(pending_jobs_mutex);
                        pending_mesh_jobs.erase(chunk_pos);
                    });

                    if (++submitted >= max_jobs_per_tick) return;
                }
            }
        }
    }

    ptr<Chunk> Main::get_or_create_chunk(const Pos<int>& chunk_pos) {
        {
            std::shared_lock lock(chunks_mutex);
            auto it = chunks.find(chunk_pos);
            if (it != chunks.end()) return it->second;
        }

        std::unique_lock lock(chunks_mutex);
        auto it = chunks.find(chunk_pos);
        if (it != chunks.end()) return it->second;

        ptr<Chunk> chunk(new Chunk());
        chunk.value().chunk_pos = chunk_pos;
        chunks[chunk_pos] = chunk;
        return chunk;
    }

    none Main::create_chunk_collision(ptr<Chunk> chunk, const PackedVector3Array& collision_faces) {
		std::shared_lock lock(chunks_mutex);

        if (not chunk.value().mesh_instance or chunk.value().collision_built.load(std::memory_order_relaxed)) return;
        
        for (int i = chunk.value().mesh_instance->get_child_count() - 1; i >= 0; --i) {
            Node* child = chunk.value().mesh_instance->get_child(i);
            if (Object::cast_to<StaticBody3D>(child)) {
                chunk.value().mesh_instance->remove_child(child);
                child->queue_free();
            }
        }

        ref<ArrayMesh> mesh = chunk.value().mesh_instance->get_mesh();
        if (mesh.is_null() or mesh->get_surface_count() == 0) return;
        if (collision_faces.size() == 0) return;

        if (collision_faces.size() % 3 != 0) {
            log<LogType::ERROR>(format{} << "Invalid faces size for collision: " << collision_faces.size());
            return;
        }

        StaticBody3D* static_body = memnew(StaticBody3D);
        CollisionShape3D* col_shape = memnew(CollisionShape3D);
        ref<ConcavePolygonShape3D> concave;
        concave.instantiate();

        concave->set_faces(collision_faces);
        col_shape->set_shape(concave);
        static_body->add_child(col_shape);

        chunk.value().mesh_instance->add_child(static_body);
        chunk.value().collision_built.store(true, std::memory_order_release);
    }
    
    none Main::update_chunk_mesh(ptr<Chunk> chunk, ref<ArrayMesh> mesh, PackedVector3Array& collision_faces) {
        if (not chunk.value().mesh_instance) {
            MeshInstance3D* mi = memnew(MeshInstance3D);
            mi->set_position(Vector3(chunk.value().chunk_pos.x * Chunk::SIZE_X, 0,chunk.value().chunk_pos.z * Chunk::SIZE_Z));
            mi->set_material_override(world_material);
            add_child(mi);
            chunk.value().mesh_instance = mi;
        }

        chunk.value().mesh_instance->set_mesh(mesh);
		create_chunk_collision(chunk, collision_faces);
    }

    none Main::unload_distant_chunks(int p_cx, int p_cz) {
        const int unload_dist = render_distance + 4;
        std::vector<Pos<int>> chunks_to_remove;

        {
            std::shared_lock lock(chunks_mutex);
            for (const auto& E : chunks) {
                int dx = std::abs(E.first.x - p_cx);
                int dz = std::abs(E.first.z - p_cz);
                if (dx > unload_dist or dz > unload_dist) {
                    chunks_to_remove.push_back(E.first);
                }
            }
        }

        {
            std::lock_guard lock(chunks_to_remove_mutex);
            this->chunks_to_remove.insert(this->chunks_to_remove.end(), chunks_to_remove.begin(), chunks_to_remove.end());
        }
        should_remove_chunks.store(true, std::memory_order_release);

        if (not chunks_to_remove.empty()) log<LogType::VERBOSE>(format{} << "Queued unload for " << chunks_to_remove.size() << " chunks.");
    }

    ptr<Chunk> Main::get_chunk(int cx, int cz) {
        std::shared_lock lock(chunks_mutex);
        Pos<int> cpos(cx, 0, cz);

        auto it = chunks.find(cpos);
        if (it == chunks.end()) return nullptr;
        return it->second;
    }
    
    uint32 Main::get_global_block_id(int wx, int wy, int wz) {
        if (wy < 0 or wy >= Chunk::SIZE_Y) return BlockRegistry::get_id("Air");

        int cx = static_cast<int>(std::floor((float32)wx / Chunk::SIZE_X));
        int cz = static_cast<int>(std::floor((float32)wz / Chunk::SIZE_Z));
        Pos<int> cpos(cx, 0, cz);

        ptr<Chunk> chunk = get_chunk(cx, cz);
        if (not chunk) return BlockRegistry::get_id("Air");

        int lx = (wx % Chunk::SIZE_X + Chunk::SIZE_X) % Chunk::SIZE_X;
        int lz = (wz % Chunk::SIZE_Z + Chunk::SIZE_Z) % Chunk::SIZE_Z;

        return chunk.value().get_block<false>({ (uint8)lx, (uint8)wy, (uint8)lz });
    }

    none Main::set_global_block_id(uint32 block_id, int wx, int wy, int wz) {
        if (wy < 0 or wy >= Chunk::SIZE_Y) return;

        int cx = static_cast<int>(std::floor((float32)wx / Chunk::SIZE_X));
        int cz = static_cast<int>(std::floor((float32)wz / Chunk::SIZE_Z));
        Pos<int> cpos(cx, 0, cz);

        ptr<Chunk> chunk = get_chunk(cx, cz);
        if (not chunk) return;

        int lx = (wx % Chunk::SIZE_X + Chunk::SIZE_X) % Chunk::SIZE_X;
        int lz = (wz % Chunk::SIZE_Z + Chunk::SIZE_Z) % Chunk::SIZE_Z;

        chunk.value().set_block({ (uint8)lx, (uint8)wy, (uint8)lz }, block_id);
    }

    none Main::save_world(const std::string& path) {
        String real_path = ProjectSettings::get_singleton()->globalize_path(path.c_str());
        std::string std_path = real_path.utf8().get_data();

        // Tạo thư mục
        std::filesystem::create_directories(std::filesystem::path(std_path).parent_path());

        std::ofstream ofs(std_path, std::ios::binary);
        if (not ofs.is_open()) {
            log<LogType::ERROR>(format{} << "Cannot open save file: " << std_path);
            return;
        }

        Player* player = static_cast<Player*>(player_ptr);
        if (not player) {
            log<LogType::ERROR>("Failed to cast to Player*");
            return;
        }

        log<LogType::INFO>("Saving world...");

        size version_len = strlen(version);
        ofs.write(reinterpret_cast<const byte*>(&version_len), sizeof(size));

        ofs.write(version, sizeof(char) * version_len);

        std::vector<std::pair<Pos<int>, ptr<Chunk>>> chunks_to_save;
        {
            std::shared_lock lock(chunks_mutex);

            uint32 seed = noise->get_seed();
            ofs.write(reinterpret_cast<const byte*>(&seed), sizeof(uint32));

            for (const auto& E : chunks) {
                if (E.second.value().generated.load(std::memory_order_acquire)) {
                    chunks_to_save.emplace_back(E.first, E.second);
                }
            }
        }

        uint32 chunk_count = static_cast<uint32>(chunks_to_save.size());
        ofs.write(reinterpret_cast<const byte*>(&chunk_count), sizeof(uint32));

        for (const auto& [pos, chunk] : chunks_to_save) {
            std::shared_lock data_lock(chunk.value().data_mutex);

            ofs.write(reinterpret_cast<const byte*>(&pos.x), sizeof(int32));
            ofs.write(reinterpret_cast<const byte*>(&pos.y), sizeof(int32));
            ofs.write(reinterpret_cast<const byte*>(&pos.z), sizeof(int32));

            const auto* data = &chunk.value().blocks[0][0][0];
            ofs.write(reinterpret_cast<const byte*>(data), Chunk::SIZE_X * Chunk::SIZE_Y * Chunk::SIZE_Z * sizeof(BlockStorage));

            uint8 block_ids_size = static_cast<uint8>(chunk.value().block_ids.size());
            ofs.write(reinterpret_cast<const byte*>(&block_ids_size), sizeof(uint8));
            for (const auto& [local_id, global_id] : chunk.value().block_ids) {
                ofs.write(reinterpret_cast<const byte*>(&local_id), sizeof(uint8));
                ofs.write(reinterpret_cast<const byte*>(&global_id), sizeof(uint32));
            }

            uint8 tag_ids_size = static_cast<uint8>(chunk.value().tag_ids.size());
            ofs.write(reinterpret_cast<const byte*>(&tag_ids_size), sizeof(uint8));
            for (const auto& [local_id, global_id] : chunk.value().tag_ids) {
                ofs.write(reinterpret_cast<const byte*>(&local_id), sizeof(uint8));
                ofs.write(reinterpret_cast<const byte*>(&global_id.first), sizeof(uint32));
                ofs.write(reinterpret_cast<const byte*>(&global_id.second), sizeof(uint16));
            }

            uint32 complex_size = static_cast<uint32>(chunk.value().complex_blocks.size());
            ofs.write(reinterpret_cast<const byte*>(&complex_size), sizeof(uint32));

            for (const auto& pair : chunk.value().complex_blocks) {
                ofs.write(reinterpret_cast<const byte*>(&pair.first.x), sizeof(uint8));
                ofs.write(reinterpret_cast<const byte*>(&pair.first.y), sizeof(uint8));
                ofs.write(reinterpret_cast<const byte*>(&pair.first.z), sizeof(uint8));

                ofs.write(reinterpret_cast<const byte*>(&pair.second.block_id), sizeof(uint32));
                ofs.write(reinterpret_cast<const byte*>(&pair.second.tag), sizeof(uint32));
            }
        }

        player->save_data(ofs);

        log<LogType::INFO>(format{} << "World saved!");
        log<LogType::VERBOSE>(format{} << chunk_count << " chunks");
    }

    bool Main::load_world(const std::string& path) {
        String real_path = ProjectSettings::get_singleton()->globalize_path(path.c_str());
        std::string std_path = real_path.utf8().get_data();

        std::ifstream ifs(std_path, std::ios::binary);
        if (not ifs.is_open()) return false;

        Player* player = static_cast<Player*>(player_ptr);
        if (not player) {
            log<LogType::ERROR>("Failed to cast to Player*");
            return false;
        }

        log<LogType::INFO>("Loading world...");

        size version_len = 0;
        ifs.read(reinterpret_cast<byte*>(&version_len), sizeof(size));

        char* current_version = new char[version_len + 1]; current_version[version_len] = '\0';
        ifs.read(current_version, sizeof(char) * version_len);
        if (strcmp(current_version, version)) log<LogType::WARNING>(format{} << "Save version" << "(" << current_version << ")" <<" mismatch with current version" << "(" << version << ")");
        delete[] current_version;

        uint32 seed = 0;
        ifs.read(reinterpret_cast<byte*>(&seed), sizeof(uint32));
        noise->set_seed(seed);
        world_seed.store(static_cast<int32>(seed), std::memory_order_release);

        uint32 chunk_count = 0;
        ifs.read(reinterpret_cast<byte*>(&chunk_count), sizeof(uint32));

        {
            std::unique_lock lock(chunks_mutex);
            chunks.clear();
        }

        for (uint32 i = 0; i < chunk_count; ++i) {
            Pos<int> pos;
            ifs.read(reinterpret_cast<byte*>(&pos.x), sizeof(int32));
            ifs.read(reinterpret_cast<byte*>(&pos.y), sizeof(int32));
            ifs.read(reinterpret_cast<byte*>(&pos.z), sizeof(int32));

            auto chunk = get_or_create_chunk(pos);
            std::unique_lock data_lock(chunk.value().data_mutex);

            uint32 size = Chunk::SIZE_X * Chunk::SIZE_Y * Chunk::SIZE_Z * sizeof(BlockStorage);
            ifs.read(reinterpret_cast<byte*>(&chunk.value().blocks[0][0][0]), size);

            uint8 block_ids_size = 0;
            ifs.read(reinterpret_cast<byte*>(&block_ids_size), sizeof(uint8));
            for (uint8 j = 0; j < block_ids_size; ++j) {
                uint8 local_id;
                uint32 global_id;
                ifs.read(reinterpret_cast<byte*>(&local_id), sizeof(uint8));
                ifs.read(reinterpret_cast<byte*>(&global_id), sizeof(uint32));
                chunk.value().block_ids[local_id] = global_id;
            }

            uint8 tag_ids_size = 0;
            ifs.read(reinterpret_cast<byte*>(&tag_ids_size), sizeof(uint8));
            for (uint8 j = 0; j < tag_ids_size; ++j) {
                uint8 local_id;
                std::pair<uint32, uint16> global_id;
                ifs.read(reinterpret_cast<byte*>(&local_id), sizeof(uint8));
                ifs.read(reinterpret_cast<byte*>(&global_id.first), sizeof(uint32));
                ifs.read(reinterpret_cast<byte*>(&global_id.second), sizeof(uint16));
                chunk.value().tag_ids[local_id] = global_id;
            }

            uint32 complex_size = 0;
            ifs.read(reinterpret_cast<byte*>(&complex_size), sizeof(uint32));

            auto& map = chunk.value().complex_blocks;
            for (uint32 j = 0; j < complex_size; ++j) {
                uint8 x, y, z;
                uint32 block_id, tag;

                ifs.read(reinterpret_cast<byte*>(&x), sizeof(uint8));
                ifs.read(reinterpret_cast<byte*>(&y), sizeof(uint8));
                ifs.read(reinterpret_cast<byte*>(&z), sizeof(uint8));

                ifs.read(reinterpret_cast<byte*>(&block_id), sizeof(uint32));
                ifs.read(reinterpret_cast<byte*>(&tag), sizeof(uint32));

                Pos<uint8> key{ x, y, z };
                BlockStorageFull value{ block_id, tag };

                chunk.value().complex_blocks.emplace(key, value);
            }

            chunk.value().generated.store(true, std::memory_order_release);
            chunk.value().dirty.store(true, std::memory_order_release);
            chunk.value().mesh_ready.store(false, std::memory_order_release);
            chunk.value().collision_built.store(false, std::memory_order_release);
        }

        player->load_data(ifs);

        log<LogType::INFO>("World loaded successfully!");
        return true;
    }

    none Main::save_userdata(const char* path) {
        String real_path = ProjectSettings::get_singleton()->globalize_path(path);
        std::string std_path = real_path.utf8().get_data();

        // Tạo thư mục
        std::filesystem::create_directories(std::filesystem::path(std_path).parent_path());

        std::ofstream ofs(std_path, std::ios::binary);
        if (not ofs.is_open()) {
            log<LogType::ERROR>(format{} << "Cannot open save file: " << std_path);
            return;
        }

        Player* player = static_cast<Player*>(player_ptr);
        if (not player) return;

        ofs.write(reinterpret_cast<const byte*>(&full_screen), sizeof(bool));
        ofs.write(reinterpret_cast<const byte*>(&player->sensitivity), sizeof(float32));
        ofs.write(reinterpret_cast<const byte*>(&player->mouse_pitch), sizeof(float32));
        ofs.write(reinterpret_cast<const byte*>(&render_distance), sizeof(int));
    }

    bool Main::load_userdata(const char* path) {
        String real_path = ProjectSettings::get_singleton()->globalize_path(path);
        std::string std_path = real_path.utf8().get_data();

        std::ifstream ifs(std_path, std::ios::binary);
        if (not ifs.is_open()) return false;

        Player* player = static_cast<Player*>(player_ptr);
        if (not player) return false;

        ifs.read(reinterpret_cast<byte*>(&full_screen), sizeof(bool));
        ifs.read(reinterpret_cast<byte*>(&player->sensitivity), sizeof(float32));
        ifs.read(reinterpret_cast<byte*>(&player->mouse_pitch), sizeof(float32));
        ifs.read(reinterpret_cast<byte*>(&render_distance), sizeof(int));

        return true;
    }

    none Main::pause() {
        Input* input = Input::get_singleton();
        input->set_mouse_mode(Input::MOUSE_MODE_VISIBLE);
        pausing.store(true, std::memory_order_relaxed);
    }
    none Main::resume() {
        Input* input = Input::get_singleton();
        input->set_mouse_mode(Input::MOUSE_MODE_CAPTURED);
        pausing.store(false, std::memory_order_relaxed);
    }

    none Main::set_seed_and_world_name(int32 seed, const String name) {
        world_seed.store(seed, std::memory_order_release);
        world_name = name.utf8();
    }

    none Main::_bind_methods() {
        ADD_SIGNAL(MethodInfo("pause"));
        ADD_SIGNAL(MethodInfo("resume"));
        ClassDB::bind_method(D_METHOD("init"), &Main::init);
        ClassDB::bind_method(D_METHOD("pause_game"), &Main::pause);
        ClassDB::bind_method(D_METHOD("resume_game"), &Main::resume);
        ClassDB::bind_method(D_METHOD("set_seed_and_world_name", "seed", "name"), &Main::set_seed_and_world_name);
    }
}
