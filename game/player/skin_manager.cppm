module;

#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

#include <includes.hpp>

export module game.player.skin_manager;

import misc.ptr;
import misc.interger;
import misc.format;
import game.logger;

using namespace godot;

export namespace craftbuild {
    class Player;
    class SkinManager {
    public:
        static bool load_skin(Player& player, const char* path);
        static ref<StandardMaterial3D> create_skin_material(ref<Texture2D> texture);
        static none apply_skin_to_model(MeshInstance3D* model, ref<Texture2D> texture);
    };
}