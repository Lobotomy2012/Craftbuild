module;

#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/classes/box_mesh.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/capsule_shape3d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include <includes.hpp>

module game.player;

import game.player.skin_manager;
import game.main;

namespace craftbuild {
    bool SkinManager::load_skin(Player& player, const char* path) {
        ref<Texture2D> skin_tex = ResourceLoader::get_singleton()->load(path);
        if (skin_tex.is_null()) {
            log<LogType::ERROR>(format{} << "Failed to load skin: " << path);
            return false;
        }

        log<LogType::INFO>(format{} << "Skin loaded: " << path);

        MeshInstance3D* player_model = player.get_node<MeshInstance3D>("mesh");
        if (player_model) {
            apply_skin_to_model(player_model, skin_tex);
        }
        else {
            log<LogType::WARNING>("Player model not found. Create a MeshInstance3D named 'Model'");
        }

        return true;
    }

    none Player::_ready() {
        // Camera
        camera = get_node<Camera3D>("camera");
        camera->set_position(Vector3(0, 1.6f, 0));

        // Model
        auto* model = get_node<MeshInstance3D>("mesh");

        ref<BoxMesh> box = memnew(BoxMesh);
        box->set_size(Vector3(0.6f, 1.8f, 0.3f));

        model->set_mesh(box);
        model->set_position(Vector3(0, 0.9f, 0));

        // Collision
        auto* collision = get_node<CollisionShape3D>("shape");

        ref<CapsuleShape3D> capsule = memnew(CapsuleShape3D);
        capsule->set_radius(0.3f);
        capsule->set_height(1.8f);

        collision->set_shape(capsule);
        collision->set_position(Vector3(0, 0.9f, 0));

        // Load Skin
        SkinManager::load_skin(*this, "res://assets/textures/skin/creeper_boy.png");
        
		// World reference
        world_ptr = Object::cast_to<Main>(get_parent());

        hotbar = { 1, 2, 3, 4, 5, 0, 0, 0, 0 };
        selected_slot = 0;

        log<LogType::INFO>("Player initialized");
    }

    none Player::_process(float64 delta) {
        if (not world_ptr) return;
        Main* world = static_cast<Main*>(world_ptr);
        if (world->pausing.load(std::memory_order_relaxed)) return;

        static bool gamemode_toggled = false;

        Input* input = Input::get_singleton();

        if (input->is_key_pressed(KEY_F3) and input->is_key_pressed(KEY_F4)) {
            if (not gamemode_toggled) {
                gamemode = (Gamemode)(((uint8_t)gamemode + 1) % 4);
                log<LogType::INFO>(format{} << "Changed gamemode to " << (int)gamemode);
                can_fly = false;
                gamemode_toggled = true;
            }
        }
        else gamemode_toggled = false;
        if (gamemode == Gamemode::Spectator) can_fly = true;
    }

    none Player::_physics_process(float64 delta) {
        if (gamemode == Gamemode::Spectator) return;
        if (not camera or not world_ptr) return;

		Main* world = static_cast<Main*>(world_ptr);
        if (world->pausing.load(std::memory_order_relaxed)) return;

        Vector3 velocity = get_velocity();
        Input* input = Input::get_singleton();

        // Gravity & Jump
        is_grounded = is_on_floor();
        if (not can_fly) {
            const float32 dt = static_cast<float32>(delta);
            const bool jump_pressed = input->is_key_pressed(KEY_SPACE);

            if (is_grounded) {
                if (velocity.y < 0.0f) velocity.y = -0.1f;
                if (jump_pressed) velocity.y = jump_velocity;
            }
            else velocity.y -= gravity * dt;
        }
        else {
            velocity.y = 0;
            if (input->is_key_pressed(KEY_SPACE)) velocity.y = speed;
            if (input->is_key_pressed(KEY_SHIFT)) velocity.y = -speed;
        }

        Vector3 forward = -camera->get_global_transform().basis.get_column(2);
        Vector3 right = camera->get_global_transform().basis.get_column(0);

        float32 forward_input = (input->is_key_pressed(KEY_W) ? 1.0f : 0.0f) - (input->is_key_pressed(KEY_S) ? 1.0f : 0.0f);
        float32 strafe_input =  (input->is_key_pressed(KEY_D) ? 1.0f : 0.0f) - (input->is_key_pressed(KEY_A) ? 1.0f : 0.0f);

        Vector3 move_dir = (forward * forward_input) + (right * strafe_input);
        move_dir.y = 0;

        if (move_dir.length() > 0) move_dir = move_dir.normalized();

        running = input->is_key_pressed(KEY_W) and (input->is_key_pressed(KEY_CTRL) or running);
        float32 current_speed = running ? speed * 2.0f : speed;

        velocity.x = move_dir.x * current_speed;
        velocity.z = move_dir.z * current_speed;

        set_velocity(velocity);
        move_and_slide();
    }

    none Player::_input(const ref<InputEvent>& event) {
        if (not camera or not world_ptr) return;
        Main* world = static_cast<Main*>(world_ptr);
        if (world->pausing.load(std::memory_order_relaxed)) return;

        // Mouse look
        if (auto mm = Object::cast_to<InputEventMouseMotion>(event.ptr())) {
            Vector2 rel = mm->get_relative();
            rotate_y(-rel.x * sensitivity);
            mouse_pitch = CLAMP(mouse_pitch - rel.y * sensitivity, -Math_PI / 2.1f, Math_PI / 2.1f);
            camera->set_rotation(Vector3(mouse_pitch, 0, 0));
        }

        // Mouse click/roll
        if (auto mb = Object::cast_to<InputEventMouseButton>(event.ptr())) {
            if (mb->is_pressed()) {
                // Mouse roll
                if (mb->get_button_index() == MOUSE_BUTTON_WHEEL_UP)   cycle_hotbar(-1);
                if (mb->get_button_index() == MOUSE_BUTTON_WHEEL_DOWN) cycle_hotbar(1);
                
                // Mouse click
                bool left = mb->get_button_index() == MOUSE_BUTTON_LEFT;
                bool right = mb->get_button_index() == MOUSE_BUTTON_RIGHT;
                if (left or right) {
                    Dictionary hit = raycast_block();
                    uint32 AIR = BlockRegistry::get_id("Air");

                    if (not hit.is_empty()) {
                        Vector3 hit_pos = hit["position"];
                        Vector3 normal = hit["normal"];

                        Vector3 break_pos_float = hit_pos - (normal * 0.001f);
                        Vector3i break_block_pos = Vector3i(break_pos_float.floor());

                        Vector3 place_pos_float = hit_pos + (normal * 0.001f);
                        Vector3i place_block_pos = Vector3i(place_pos_float.floor());
                        uint32 target_block_id = world->get_global_block_id(break_block_pos.x, break_block_pos.y, break_block_pos.z);

                        log<LogType::INFO>(format{} << "Looking at block id: " << target_block_id << " at (" << break_block_pos.x << ", " << break_block_pos.y << ", " << break_block_pos.z << ")");
                        
                        if (left)  world->set_global_block_id(AIR, break_block_pos.x, break_block_pos.y, break_block_pos.z);
                        if (right) world->set_global_block_id(get_selected_block_id(), break_block_pos.x, break_block_pos.y, break_block_pos.z);

                        const int cx = static_cast<int>(std::floor((float32)break_block_pos.x / Chunk::SIZE_X));
                        const int cz = static_cast<int>(std::floor((float32)break_block_pos.z / Chunk::SIZE_Z));
                        if (auto chunk = world->get_chunk(cx, cz)) {
                            chunk.value().dirty.store(true, std::memory_order_release);
                            chunk.value().mesh_ready.store(false, std::memory_order_release);
                            chunk.value().collision_built.store(false, std::memory_order_release);

                            Pos<int> neighbor_offsets[4] = { {1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1} };
                            ptr<Chunk> neighbor = nullptr;
                            for (const auto& offset : neighbor_offsets) {
                                if (neighbor = world->get_chunk(cx + offset.x, cz + offset.z)) break;
                            }

                            if (not neighbor) return;
                            neighbor.value().dirty.store(true, std::memory_order_release);
                            neighbor.value().mesh_ready.store(false, std::memory_order_release);
                            neighbor.value().collision_built.store(false, std::memory_order_release);
                        }
                    }
                }
            }
        }

        for (int i = 0; i < 9; ++i) {
            Key key = (Key)(KEY_1 + i);
            if (Input::get_singleton()->is_key_pressed(key)) {
                if (event->is_pressed() and not event->is_echo()) {
                    select_slot(i);
                }
            }
        }
    }

    Dictionary Player::raycast_block(float max_distance) {
        if (not camera) return Dictionary();

        Vector2 screen_center = camera->get_viewport()->get_visible_rect().get_center();
        Vector3 origin = camera->project_ray_origin(screen_center);
        Vector3 direction = camera->project_ray_normal(screen_center);
        Vector3 end = origin + direction * max_distance;

        PhysicsDirectSpaceState3D* space_state = camera->get_world_3d()->get_direct_space_state();
        Ref<PhysicsRayQueryParameters3D> query = PhysicsRayQueryParameters3D::create(origin, end);

        query->set_collision_mask(1); 

        return space_state->intersect_ray(query);
    }

    none Player::cycle_hotbar(int dir) {
        selected_slot = (selected_slot + dir + HOTBAR_SIZE) % HOTBAR_SIZE;
        log<LogType::INFO>(format{} << "Selected slot: " << selected_slot + 1);
    }

    none Player::select_slot(int slot) {

    }

    none Player::save_data(std::ostream& os) {
        os.write(reinterpret_cast<const byte*>(&speed), sizeof(float32));
        os.write(reinterpret_cast<const byte*>(&gravity), sizeof(float32));
        os.write(reinterpret_cast<const byte*>(&jump_velocity), sizeof(float32));
        os.write(reinterpret_cast<const byte*>(&is_grounded), sizeof(bool));
        os.write(reinterpret_cast<const byte*>(&can_fly), sizeof(bool));
        os.write(reinterpret_cast<const byte*>(&running), sizeof(bool));
        os.write(reinterpret_cast<const byte*>(&gamemode), sizeof(Gamemode));
        os.write(reinterpret_cast<const byte*>(&hotbar), sizeof(uint32) * HOTBAR_SIZE);
        os.write(reinterpret_cast<const byte*>(&selected_slot), sizeof(uint8));
    }

    none Player::load_data(std::istream& is) {
        is.read(reinterpret_cast<byte*>(&speed), sizeof(float32));
        is.read(reinterpret_cast<byte*>(&gravity), sizeof(float32));
        is.read(reinterpret_cast<byte*>(&jump_velocity), sizeof(float32));
        is.read(reinterpret_cast<byte*>(&is_grounded), sizeof(bool));
        is.read(reinterpret_cast<byte*>(&can_fly), sizeof(bool));
        is.read(reinterpret_cast<byte*>(&running), sizeof(bool));
        is.read(reinterpret_cast<byte*>(&gamemode), sizeof(Gamemode));
        is.read(reinterpret_cast<byte*>(&hotbar), sizeof(uint32) * HOTBAR_SIZE);
        is.read(reinterpret_cast<byte*>(&selected_slot), sizeof(uint8));
    }

    none Player::_bind_methods() {}
}
