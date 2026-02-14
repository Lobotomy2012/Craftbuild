#pragma once

#include <core.hpp>
#include <Graphics/variable.hpp>
#include <Player/enum.hpp>
#include <World/class.hpp>
#include <Player/variable.hpp>
#include <Graphics/function.hpp>

namespace Craftbuild {
    struct Player {
        float speed = 8.0f;
        float yaw = -90.0f;
        float pitch = 0.0f;
        float last_x = WIDTH / 2.0f;
        float last_y = HEIGHT / 2.0f;
        bool first_mouse = true;

        float gravity = 32.0f;
        float vertical_velocity = 0.0f;
        bool is_grounded = false;
        bool can_fly = false;

        bool show_f3_screen = false;

        glm::vec3 pos = glm::vec3(0.0f, 15.0f, 0.0f);
        glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);

		Gamemode gamemode = Gamemode::Creative;

        inline void scroll_callback(double xoffset, double yoffset) {
            if (gamemode == Gamemode::Spectator) {
                speed += yoffset * 2.0f;
                if (speed < 1.0f) speed = 1.0f;
                if (speed > 50.0f) speed = 50.0f;
            }

        }

        bool raycast_block(const std::unordered_map<int64_t, Chunk>& chunk_map, glm::ivec3& out_block, glm::ivec3& out_adjacent, float max_distance = 8.0f, float step = 0.1f) const {
            glm::vec3 origin = pos;
            glm::vec3 dir = glm::normalize(camera_front);

            glm::ivec3 last_block = glm::ivec3(glm::floor(origin));

            for (float t = 0.0f; t <= max_distance; t += step) {
                glm::vec3 p = origin + dir * t;
                int bx = static_cast<int>(std::floor(p.x));
                int by = static_cast<int>(std::floor(p.y));
                int bz = static_cast<int>(std::floor(p.z));

                if (by < 0 or by >= WORLD_HEIGHT) continue;

                glm::ivec3 current_block(bx, by, bz);
                if (current_block == last_block) continue;
                last_block = current_block;

                int chunk_x = static_cast<int>(std::floor(bx / 16.0f));
                int chunk_z = static_cast<int>(std::floor(bz / 16.0f));
                int local_x = ((bx % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;
                int local_z = ((bz % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;

                auto it = chunk_map.find(make_key(chunk_x, chunk_z));
                if (it == chunk_map.end()) continue;

                BlockType b = it->second.blocks[local_x][by][local_z];
                if (b != BlockType::AIR && b != BlockType::WATER) {
                    out_block = current_block;

                    glm::vec3 prev_p = origin + dir * std::max(0.0f, t - step);
                    int pbx = static_cast<int>(std::floor(prev_p.x));
                    int pby = static_cast<int>(std::floor(prev_p.y));
                    int pbz = static_cast<int>(std::floor(prev_p.z));
                    glm::ivec3 prev_block(pbx, pby, pbz);
                    glm::ivec3 normal = out_block - prev_block;

                    if (normal == glm::ivec3(0)) {
                        glm::vec3 local = p - (glm::vec3(bx, by, bz) + glm::vec3(0.5f));
                        glm::vec3 abs_local = glm::abs(local);
                        if (abs_local.x >= abs_local.y && abs_local.x >= abs_local.z) normal = glm::ivec3((local.x > 0) ? 1 : -1, 0, 0);
                        else if (abs_local.y >= abs_local.x && abs_local.y >= abs_local.z) normal = glm::ivec3(0, (local.y > 0) ? 1 : -1, 0);
                        else normal = glm::ivec3(0, 0, (local.z > 0) ? 1 : -1);
                    }

                    out_adjacent = out_block + normal;
                    return true;
                }
            }

            return false;
        }

        bool is_block_at_position(float x, float y, float z, const std::unordered_map<int64_t, Chunk>& chunk_map) {
            int chunk_x = static_cast<int>(std::floor(x / 16.0f));
            int chunk_z = static_cast<int>(std::floor(z / 16.0f));

            int local_x = ((static_cast<int>(std::floor(x)) % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;
            int local_z = ((static_cast<int>(std::floor(z)) % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;
            int block_y = static_cast<int>(std::floor(y - 1.8f));

            auto it = chunk_map.find(make_key(chunk_x, chunk_z));
            if (it == chunk_map.end()) return false;
            if (block_y < 0 or block_y >= WORLD_HEIGHT) return false;

            BlockType b = it->second.blocks[local_x][block_y][local_z];
            return (b != BlockType::AIR and b != BlockType::WATER);
        }

        bool check_collision(const glm::vec3& new_pos, const std::unordered_map<int64_t, Chunk>& chunk_map) {
            const float HALF_WIDTH = PLAYER_WIDTH / 2.0f;

            for (float dx = -HALF_WIDTH; dx <= HALF_WIDTH; dx += HALF_WIDTH) {
                for (float dy = 0.0f; dy <= PLAYER_HEIGHT; dy += PLAYER_HEIGHT / 2.0f) {
                    for (float dz = -HALF_WIDTH; dz <= HALF_WIDTH; dz += HALF_WIDTH) {
                        float check_x = new_pos.x + dx;
                        float check_y = new_pos.y + dy;
                        float check_z = new_pos.z + dz;

                        if (is_block_at_position(check_x, check_y, check_z, chunk_map)) {
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        glm::vec3 move_with_collision(const glm::vec3& direction, float velocity,
            const std::unordered_map<int64_t, Chunk>& chunk_map) {
            glm::vec3 final_pos = pos;

            // X
            glm::vec3 test_pos_x = pos;
            test_pos_x.x += direction.x * velocity;
            if (!check_collision(test_pos_x, chunk_map)) {
                final_pos.x = test_pos_x.x;
            }

            // Z
            glm::vec3 test_pos_z = final_pos;
            test_pos_z.z += direction.z * velocity;
            if (!check_collision(test_pos_z, chunk_map)) {
                final_pos.z = test_pos_z.z;
            }

            return final_pos;
        }

        void update_gravity(float delta_time, const std::unordered_map<int64_t, Chunk>& chunk_map) {
            if (!is_grounded) {
                vertical_velocity -= gravity * delta_time;
                pos.y += vertical_velocity * delta_time;
            }

            int chunk_x = static_cast<int>(std::floor(pos.x / 16.0f));
            int chunk_z = static_cast<int>(std::floor(pos.z / 16.0f));

            int local_x = ((static_cast<int>(std::floor(pos.x)) % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;
            int local_z = ((static_cast<int>(std::floor(pos.z)) % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;

            auto it = chunk_map.find(make_key(chunk_x, chunk_z));
            if (it == chunk_map.end()) return;

            int block_y = static_cast<int>(std::floor(pos.y - 1.8f));

            if (block_y < 0 or block_y >= WORLD_HEIGHT) return;

            const BlockType below = it->second.blocks[local_x][block_y][local_z];
            if (below != BlockType::AIR and below != BlockType::WATER) {
                pos.y = block_y + 2.8f;
                vertical_velocity = 0;
                is_grounded = true;
				can_fly = false;
            }
            else {
                is_grounded = false;
            }
        }

        void update_player(float delta_time, float current_frame, bool keys[], const std::unordered_map<int64_t, Chunk>& chunk_map) {
            float velocity = speed * FIXED_DT;
            static bool gamemode_toggled = false;
            static bool f3_toggled = false;
            static double last_press_space_time = 0.0;
            static bool released_since_last_press_space = true;
            static auto run_velocity = velocity;

            if (keys[GLFW_KEY_F3] and keys[GLFW_KEY_F4]) {
                if (!gamemode_toggled) {
                    gamemode = (Gamemode)(((uint8_t)gamemode + 1) % 4);
                    std::cout << "\033[96m[Game]\033[36m Changed gamemode to " << (int)gamemode << "\n";
                    can_fly = false;
                    gamemode_toggled = true;
                }
            }
            else if (keys[GLFW_KEY_F3]) {
                if (!f3_toggled and show_f3_screen and enable_validation_layers) {
                    std::cout << "\033[96m[Game]\033[36m Player position: (" << pos.x << ", " << pos.y << ", " << pos.z << ")\n";
                    std::cout << "\033[96m[Game]\033[36m FPS: " << 1.0f / delta_time << "\n";
                    f3_toggled = true;
                }
                show_f3_screen = !show_f3_screen;
                gamemode_toggled = false;
            }
            else {
                f3_toggled = false;
                gamemode_toggled = false;
            }
            
            if (gamemode != Gamemode::Spectator) {
                auto camera_front_flat = glm::normalize(glm::vec3(camera_front.x, 0.0f, camera_front.z));

                if (keys[GLFW_KEY_LEFT_SHIFT] and can_fly) pos -= velocity * camera_up;
                else if (keys[GLFW_KEY_LEFT_SHIFT]) velocity /= 2;

                if (keys[GLFW_KEY_SPACE] and can_fly) pos += velocity * camera_up;
                else if (keys[GLFW_KEY_SPACE] and is_grounded) {
                    vertical_velocity = 8.5f;
                    is_grounded = false;
                }
                
                if (keys[GLFW_KEY_W]) {
                    if (keys[GLFW_KEY_LEFT_CONTROL] and (is_grounded or can_fly)) {
                        run_velocity += (velocity * 2 - run_velocity) * 0.4f;
                    }
                    pos = move_with_collision(camera_front_flat, run_velocity, chunk_map);
                }
                else run_velocity = velocity;
                if (keys[GLFW_KEY_A]) {
                    glm::vec3 left = -glm::normalize(glm::cross(camera_front, camera_up));
                    pos = move_with_collision(left, velocity, chunk_map);
                }
                if (keys[GLFW_KEY_D]) {
                    glm::vec3 right = glm::normalize(glm::cross(camera_front, camera_up));
                    pos = move_with_collision(right, velocity, chunk_map);
                }

                if (keys[GLFW_KEY_S])
                    pos = move_with_collision(-camera_front_flat, velocity, chunk_map);

                if (!can_fly) update_gravity(FIXED_DT, chunk_map);

                // Double tap space
                if (keys[GLFW_KEY_SPACE] and gamemode == Gamemode::Creative) {
                    if (current_frame - last_press_space_time < DOUBLE_TAP_THRESHOLD and released_since_last_press_space) can_fly = !can_fly;

                    last_press_space_time = current_frame;
                    released_since_last_press_space = false;
                }
                else released_since_last_press_space = true;
            }
            else {
                if (keys[GLFW_KEY_SPACE]) pos += velocity * camera_up;
                if (keys[GLFW_KEY_LEFT_SHIFT]) pos -= velocity * camera_up;

                if (keys[GLFW_KEY_W]) {
                    if (keys[GLFW_KEY_LEFT_CONTROL]) velocity *= 2;
                    pos += velocity * camera_front;
                }
                if (keys[GLFW_KEY_S]) pos -= velocity * camera_front;
                if (keys[GLFW_KEY_A]) pos -= glm::normalize(glm::cross(camera_front, camera_up)) * velocity;
                if (keys[GLFW_KEY_D]) pos += glm::normalize(glm::cross(camera_front, camera_up)) * velocity;
            }
        }

        friend std::ostream& operator<<(std::ostream& os, const Player& player) {
            os.write(reinterpret_cast<const char*>(&player.speed), sizeof(player.speed));
            os.write(reinterpret_cast<const char*>(&player.yaw), sizeof(player.yaw));
            os.write(reinterpret_cast<const char*>(&player.pitch), sizeof(player.pitch));
            os.write(reinterpret_cast<const char*>(&player.last_x), sizeof(player.last_x));
            os.write(reinterpret_cast<const char*>(&player.last_y), sizeof(player.last_y));
            os.write(reinterpret_cast<const char*>(&player.first_mouse), sizeof(player.first_mouse));
            os.write(reinterpret_cast<const char*>(&player.gravity), sizeof(player.gravity));
            os.write(reinterpret_cast<const char*>(&player.vertical_velocity), sizeof(player.vertical_velocity));
            os.write(reinterpret_cast<const char*>(&player.is_grounded), sizeof(player.is_grounded));
            os.write(reinterpret_cast<const char*>(&player.can_fly), sizeof(player.can_fly));
            os.write(reinterpret_cast<const char*>(&player.show_f3_screen), sizeof(player.show_f3_screen));
            os.write(reinterpret_cast<const char*>(&player.pos.x), sizeof(player.pos.x));
            os.write(reinterpret_cast<const char*>(&player.pos.y), sizeof(player.pos.y));
            os.write(reinterpret_cast<const char*>(&player.pos.z), sizeof(player.pos.z));
            os.write(reinterpret_cast<const char*>(&player.camera_front.x), sizeof(player.camera_front.x));
            os.write(reinterpret_cast<const char*>(&player.camera_front.y), sizeof(player.camera_front.y));
            os.write(reinterpret_cast<const char*>(&player.camera_front.z), sizeof(player.camera_front.z));
            os.write(reinterpret_cast<const char*>(&player.camera_up.x), sizeof(player.camera_up.x));
            os.write(reinterpret_cast<const char*>(&player.camera_up.y), sizeof(player.camera_up.y));
            os.write(reinterpret_cast<const char*>(&player.camera_up.z), sizeof(player.camera_up.z));
            os.write(reinterpret_cast<const char*>(&player.gamemode), sizeof(player.gamemode));
            return os;
		}

        friend std::istream& operator>>(std::istream& is, Player& player) {
            is.read(reinterpret_cast<char*>(&player.speed), sizeof(player.speed));
            is.read(reinterpret_cast<char*>(&player.yaw), sizeof(player.yaw));
            is.read(reinterpret_cast<char*>(&player.pitch), sizeof(player.pitch));
            is.read(reinterpret_cast<char*>(&player.last_x), sizeof(player.last_x));
            is.read(reinterpret_cast<char*>(&player.last_y), sizeof(player.last_y));
            is.read(reinterpret_cast<char*>(&player.first_mouse), sizeof(player.first_mouse));
            is.read(reinterpret_cast<char*>(&player.gravity), sizeof(player.gravity));
            is.read(reinterpret_cast<char*>(&player.vertical_velocity), sizeof(player.vertical_velocity));
            is.read(reinterpret_cast<char*>(&player.is_grounded), sizeof(player.is_grounded));
            is.read(reinterpret_cast<char*>(&player.can_fly), sizeof(player.can_fly));
            is.read(reinterpret_cast<char*>(&player.show_f3_screen), sizeof(player.show_f3_screen));
            is.read(reinterpret_cast<char*>(&player.pos.x), sizeof(player.pos.x));
            is.read(reinterpret_cast<char*>(&player.pos.y), sizeof(player.pos.y));
            is.read(reinterpret_cast<char*>(&player.pos.z), sizeof(player.pos.z));
            is.read(reinterpret_cast<char*>(&player.camera_front.x), sizeof(player.camera_front.x));
            is.read(reinterpret_cast<char*>(&player.camera_front.y), sizeof(player.camera_front.y));
            is.read(reinterpret_cast<char*>(&player.camera_front.z), sizeof(player.camera_front.z));
            is.read(reinterpret_cast<char*>(&player.camera_up.x), sizeof(player.camera_up.x));
            is.read(reinterpret_cast<char*>(&player.camera_up.y), sizeof(player.camera_up.y));
            is.read(reinterpret_cast<char*>(&player.camera_up.z), sizeof(player.camera_up.z));
            is.read(reinterpret_cast<char*>(&player.gamemode), sizeof(player.gamemode));
            return is;
		}
    };
}