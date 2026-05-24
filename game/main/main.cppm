module;

#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>

#include <includes.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <deque>
#include <memory>

export module game.main;

import misc.types;
import misc.format;
import misc.ptr;
import game.pos;
import game.core;
import game.block;
import game.logger;
import game.thread;
import game.world.chunk;
import game.world.biome;
import game.block.normal_blocks;
import game.texture.atlas_texture;

using namespace godot;

export namespace craftbuild {
    class Main : public Node3D {
        GDCLASS(Main, Node3D)

    private:
        std::unordered_map<Pos<int>, ptr<Chunk>, PosHash<int>> chunks;
        mutable std::shared_mutex chunks_mutex;

        ref<FastNoiseLite> noise;
        ref<ShaderMaterial> world_material;
        std::atomic<int32> world_seed = 123456789;

        none* player_ptr = nullptr;

        std::atomic<bool> running = true;
        std::atomic<bool> world_ready = false;
        std::atomic<float32> player_x = 0;
        std::atomic<float32> player_y = 0;
        std::atomic<float32> player_z = 0;
        std::thread log_thread;
        std::thread terrain_thread;
        std::thread mesh_thread;
        std::thread redstone_thread;

		std::vector<Pos<int>> chunks_to_remove;
        std::mutex chunks_to_remove_mutex;
        std::atomic<bool> should_remove_chunks = false;
        std::atomic<bool> pausing = true;

        bool full_screen = false;

    public:
        inline static int render_distance = 32;

        inline static const int SIZE_X = render_distance * 16;
        inline static const int SIZE_Z = render_distance * 16;

        none _ready() override;
        none _process(float64 delta) override;
        none _notification(int p_what);
        none _input(const ref<InputEvent>& event) override;
        
        none setup_voxel_material();

        none start_log_thread();
        none start_terrain_thread();
        none start_mesh_thread();
        none start_redstone_thread();
        bool should_render_face(ptr<Chunk> chunk, ptr<Chunk> neighbors[4], const Pos<int>& npos);
        ptr<Chunk> get_or_create_chunk(const Pos<int>& chunk_pos);
        none create_chunk_collision(ptr<Chunk> chunk, const PackedVector3Array& collision_faces);
        none generate_mesh(ptr<Chunk> chunk);
        none update_chunk_mesh(ptr<Chunk> chunk, ref<ArrayMesh> mesh, PackedVector3Array& collision_faces);
        none unload_distant_chunks(int p_cx, int p_cz);

        ptr<Chunk> get_chunk(int cx, int cz);
        uint32 get_global_block_id(int wx, int wy, int wz);
        none set_global_block_id(uint32 block_id, int wx, int wy, int wz);

        none save_world(const char* path = "res://saves/overworld.cbsave");
        bool load_world(const char* path = "res://saves/overworld.cbsave");

        none save_userdata(const char* path = "res://userdata.cbdata");
        bool load_userdata(const char* path = "res://userdata.cbdata");

        none pause();
        none resume();

        static none _bind_methods();

        friend class Player;
    };
}
