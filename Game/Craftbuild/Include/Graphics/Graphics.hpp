#pragma once

#include <core.hpp>
#include <World/world.hpp>
#include <Graphics/variable.hpp>
#include <Graphics/struct.hpp>
#include <Graphics/function.hpp>

namespace Craftbuild {
    class CraftbuildGraphics {
    public:
        CraftbuildGraphics() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(1, 1'000'000'000);
		    const int seed = dist(gen);
            std::cerr << "\033[96m[Game]\033[36m Seed set to " << seed << "\n";
            world.set_seed(seed);

            init_window();
            init_vulkan();
            std::cerr << "\033[96m[Game]\033[36m Generating World\n";
            generate_world_mesh();
            std::cerr << "\033[96m[Game]\033[36m Done! Have fun\n";
            init_input();
            main_loop();
        }
        ~CraftbuildGraphics() {
            cleanup_swap_chain();

            vkDestroyPipeline(device, graphics_pipeline, nullptr);
            vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
            vkDestroyRenderPass(device, render_pass, nullptr);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkUnmapMemory(device, uniform_buffers_memory[i]);
                vkDestroyBuffer(device, uniform_buffers[i], nullptr);
                vkFreeMemory(device, uniform_buffers_memory[i], nullptr);
            }

            vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
            vkDestroySampler(device, texture_sampler, nullptr);

            // Cleanup all textures
            for (auto& [type, image_view] : block_texture_views) {
                vkDestroyImageView(device, image_view, nullptr);
            }
            for (auto& [type, image] : block_textures) {
                vkDestroyImage(device, image, nullptr);
            }
            for (auto& [type, memory] : block_textures_memory) {
                vkFreeMemory(device, memory, nullptr);
            }

            // Cleanup texture atlas
            vkDestroyImageView(device, texture_atlas_view, nullptr);
            vkDestroyImage(device, texture_atlas_image, nullptr);
            vkFreeMemory(device, texture_atlas_memory, nullptr);

            vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
            vkDestroyBuffer(device, vertex_buffer, nullptr);
            vkFreeMemory(device, vertex_buffer_memory, nullptr);
            vkDestroyBuffer(device, index_buffer, nullptr);
            vkFreeMemory(device, index_buffer_memory, nullptr);

            for (size_t i = 0; i < image_available_semaphores.size(); i++) {
                vkDestroySemaphore(device, image_available_semaphores[i], nullptr);
                vkDestroySemaphore(device, render_finished_semaphores[i], nullptr);
            }
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkDestroyFence(device, in_flight_fences[i], nullptr);
            }

            vkDestroyCommandPool(device, command_pool, nullptr);
            vkDestroyDevice(device, nullptr);

            if (enable_validation_layers) {
                destroy_debug_utils_messenger_ext(instance, debug_messenger, nullptr);
            }

            vkDestroySurfaceKHR(instance, surface, nullptr);
            vkDestroyInstance(instance, nullptr);

            chunk_map.clear();

            glfwDestroyWindow(window);
            glfwTerminate();
        }
    private:
        GLFWwindow* window;
        VkInstance instance;
        VkDebugUtilsMessengerEXT debug_messenger;
        VkSurfaceKHR surface;
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        VkSampleCountFlagBits msaa_samples = VK_SAMPLE_COUNT_1_BIT;
        VkDevice device;
        VkQueue graphics_queue;
        VkQueue present_queue;
        VkSwapchainKHR swap_chain;

        std::vector<VkImage> swap_chain_images;

        VkFormat swap_chain_image_format;
        VkExtent2D swap_chain_extent;

        std::vector<VkImageView> swap_chain_image_views;
        std::vector<VkFramebuffer> swap_chain_framebuffers;

        VkRenderPass render_pass;
        VkDescriptorSetLayout descriptor_set_layout;
        VkPipelineLayout pipeline_layout;
        VkPipeline graphics_pipeline;
        VkCommandPool command_pool;
        VkImage color_image;
        VkDeviceMemory color_image_memory;
        VkImageView color_image_view;
        VkImage depth_image;
        VkDeviceMemory depth_image_memory;
        VkImageView depth_image_view;
        VkSampler texture_sampler;

        VkBuffer vertex_buffer;
        VkDeviceMemory vertex_buffer_memory;
        VkBuffer index_buffer;
        VkDeviceMemory index_buffer_memory;
        VkDeviceSize current_index_buffer_size = 0;
        VkDeviceSize current_vertex_buffer_size = 0;
        size_t all_vertices_size = 0;
        size_t all_indices_size = 0;

        std::vector<VkBuffer> uniform_buffers;
        std::vector<VkDeviceMemory> uniform_buffers_memory;
        std::vector<void*> uniform_buffers_mapped;
        VkDescriptorPool descriptor_pool;

        std::vector<VkDescriptorSet> descriptor_sets;
        std::vector<VkCommandBuffer> command_buffers;
        std::vector<VkSemaphore> image_available_semaphores;
        std::vector<VkSemaphore> render_finished_semaphores;
        std::vector<VkFence> in_flight_fences;
        std::vector<VkFence> images_in_flight;

        uint32_t current_frame = 0;
        bool framebuffer_resized = false;
        bool escape_pressed_last_frame = false;
        World world;
        
        std::unordered_map<int64_t, Chunk> chunk_map;
        glm::vec3 camera_pos = glm::vec3(0.0f, 70.0f, 0.0f);
        glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);

        float camera_speed = 10.0f;
        float yaw = -90.0f;
        float pitch = 0.0f;
        float delta_time = 0.0f;
        float last_frame = 0.0f;
        bool keys[1024] = { false };
        bool first_mouse = true;
        float last_x = WIDTH / 2.0f;
        float last_y = HEIGHT / 2.0f;
        bool pause = false;

        std::unordered_map<BlockType, VkImage> block_textures;
        std::unordered_map<BlockType, VkDeviceMemory> block_textures_memory;
        std::unordered_map<BlockType, VkImageView> block_texture_views;

        VkImage texture_atlas_image;
        VkDeviceMemory texture_atlas_memory;
        VkImageView texture_atlas_view;

        std::mutex mesh_mutex;
        std::atomic<bool> mesh_ready = false;
        std::atomic<bool> generating_mesh = false;
        std::vector<Vertex> pending_vertices;
        std::vector<uint32_t> pending_indices;

        void init_window() {
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            window = glfwCreateWindow(WIDTH, HEIGHT, "Craftbuild indev 1.0", nullptr, nullptr);
            glfwSetWindowUserPointer(window, this);
            glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
        }

        static void framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
            auto app = reinterpret_cast<CraftbuildGraphics*>(glfwGetWindowUserPointer(window));
            app->framebuffer_resized = true;
        }

        void init_vulkan() {
            create_instance();
            setup_debug_messenger();
            create_surface();
            pick_physical_device();
            create_logical_device();
            create_swap_chain();
            create_image_views();
            create_render_pass();
            create_descriptor_set_layout();
            create_graphics_pipeline();
            create_command_pool();
            create_color_resources();
            create_depth_resources();
            create_framebuffers();
            load_all_textures();
            create_texture_sampler();
            create_uniform_buffers();
            create_descriptor_pool();
            create_descriptor_sets();
            create_command_buffers();
            create_sync_objects();
        }

        void init_input() {
            glfwSetKeyCallback(window, key_callback);
            glfwSetCursorPosCallback(window, mouse_callback);
            glfwSetScrollCallback(window, scroll_callback);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
            CraftbuildGraphics* app = reinterpret_cast<CraftbuildGraphics*>(glfwGetWindowUserPointer(window));
            if (action == GLFW_PRESS) {
                app->keys[key] = true;
            }
            else if (action == GLFW_RELEASE) {
                app->keys[key] = false;
            }
        }

        static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
            CraftbuildGraphics* app = reinterpret_cast<CraftbuildGraphics*>(glfwGetWindowUserPointer(window));
            if (app->first_mouse) {
                app->last_x = xpos;
                app->last_y = ypos;
                app->first_mouse = false;
            }

            float xoffset = xpos - app->last_x;
            float yoffset = app->last_y - ypos;

            app->last_x = xpos;
            app->last_y = ypos;

            float sensitivity = 0.1f;

            xoffset *= sensitivity;
            yoffset *= sensitivity;

            app->yaw += xoffset;
            app->pitch += yoffset;

            // make sure that when pitch is out of bounds
            if (app->pitch > 89.0f)
                app->pitch = 89.0f;
            if (app->pitch < -89.0f)
                app->pitch = -89.0f;
            glm::vec3 front;
            front.x = cos(glm::radians(app->yaw)) * cos(glm::radians(app->pitch));
            front.y = sin(glm::radians(app->pitch));
            front.z = sin(glm::radians(app->yaw)) * cos(glm::radians(app->pitch));
            app->camera_front = glm::normalize(front);
        }

        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
            CraftbuildGraphics* app = reinterpret_cast<CraftbuildGraphics*>(glfwGetWindowUserPointer(window));
            app->camera_speed += yoffset * 2.0f;
            if (app->camera_speed < 1.0f) app->camera_speed = 1.0f;
            if (app->camera_speed > 50.0f) app->camera_speed = 50.0f;
        }

        void process_input() {
            if (keys[GLFW_KEY_ESCAPE]) {
                if (!escape_pressed_last_frame) {
                    pause = not pause;
                    glfwSetInputMode(window, GLFW_CURSOR, pause ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
                    escape_pressed_last_frame = true;
                }
            }
            else {
                escape_pressed_last_frame = false;
            }
            if (pause) return;

            float _camera_speed = camera_speed * delta_time;

            if (keys[GLFW_KEY_W]) {
                if (keys[GLFW_KEY_LEFT_CONTROL]) _camera_speed *= 2;
                camera_pos += _camera_speed * camera_front;
            }
            if (keys[GLFW_KEY_S]) camera_pos -= _camera_speed * camera_front;
            if (keys[GLFW_KEY_A]) camera_pos -= glm::normalize(glm::cross(camera_front, camera_up)) * _camera_speed;
            if (keys[GLFW_KEY_D]) camera_pos += glm::normalize(glm::cross(camera_front, camera_up)) * _camera_speed;
            if (keys[GLFW_KEY_SPACE]) camera_pos += _camera_speed * camera_up;
            if (keys[GLFW_KEY_LEFT_SHIFT]) camera_pos -= _camera_speed * camera_up;
        }

        void generate_world_mesh() {
            std::lock_guard<std::mutex> lock(mesh_mutex);

            if (generating_mesh.load()) return;
            generating_mesh.store(true);

            int player_chunk_x = static_cast<int>(camera_pos.x) / 16;
            int player_chunk_z = static_cast<int>(camera_pos.z) / 16;
            constexpr int render_distance = 8;
            // Build set of desired chunk keys within render distance
            auto make_key = [](int x, int z) -> int64_t {
                return (static_cast<int64_t>(x) << 32) | static_cast<uint32_t>(z);
            };

            std::unordered_set<int64_t> desired_keys;
            desired_keys.reserve((render_distance * 2 + 1) * (render_distance * 2 + 1));
            for (int dx = -render_distance; dx <= render_distance; ++dx) {
                for (int dz = -render_distance; dz <= render_distance; ++dz) {
                    int cx = player_chunk_x + dx;
                    int cz = player_chunk_z + dz;
                    desired_keys.insert(make_key(cx, cz));
                }
            }

            // Remove chunks
            std::vector<int64_t> to_remove;
            to_remove.reserve(chunk_map.size());
            for (const auto& [key, chunk] : chunk_map) {
                if (desired_keys.find(key) == desired_keys.end()) {
                    to_remove.push_back(key);
                }
            }
            for (auto key : to_remove) {
                chunk_map.erase(key);
            }

            // Add chunks
            for (int dx = -render_distance; dx <= render_distance; ++dx) {
                for (int dz = -render_distance; dz <= render_distance; ++dz) {
                    int cx = player_chunk_x + dx;
                    int cz = player_chunk_z + dz;
                    int64_t key = make_key(cx, cz);
                    if (chunk_map.find(key) == chunk_map.end()) {
                        Chunk nc;
                        nc.x = cx;
                        nc.z = cz;
                        nc.generate(cx, cz);
                        world.generate_chunk(nc);
                        chunk_map.emplace(key, std::move(nc));
                    }
                }
            }

            std::vector<Chunk*> chunk_ptrs;
            chunk_ptrs.reserve(chunk_map.size());
            for (auto& [key, chunk] : chunk_map) chunk_ptrs.push_back(&chunk);

            std::atomic<size_t> idx{ 0 };
            std::atomic<size_t> vertices_sum{ 0 };
            std::atomic<size_t> indices_sum{ 0 };
            std::atomic<size_t> generated_count{ 0 };

            unsigned int thread_count = std::max(1u, std::thread::hardware_concurrency());
            std::vector<std::thread> threads;

            auto worker = [&]() {
                while (true) {
                    size_t i = idx.fetch_add(1);
                    if (i >= chunk_ptrs.size()) break;
                    Chunk* chunk = chunk_ptrs[i];
                    if (chunk->vertices.empty()) {
                        create_chunk_mesh(*chunk);
                        generated_count.fetch_add(1);
                    }
                    vertices_sum.fetch_add(chunk->vertices.size());
                    indices_sum.fetch_add(chunk->indices.size());
                }
            };

            for (unsigned int t = 0; t < thread_count; ++t) threads.emplace_back(worker);
            for (auto& t : threads) t.join();

            size_t total_generated = generated_count.load();

            std::vector<Vertex> combined_vertices;
            std::vector<uint32_t> combined_indices;
            combined_vertices.reserve(vertices_sum.load());
            combined_indices.reserve(indices_sum.load());

            uint32_t vertex_offset = 0;
            std::vector<Chunk*> chunks;
            chunks.reserve(chunk_map.size());
            for (auto& [k, c] : chunk_map) chunks.push_back(&c);
            std::sort(chunks.begin(), chunks.end(), [&](Chunk* a, Chunk* b) {
                int ax = a->x, az = a->z;
                int bx = b->x, bz = b->z;
                int px = static_cast<int>(camera_pos.x) / 16;
                int pz = static_cast<int>(camera_pos.z) / 16;
                return (std::abs(ax - px) + std::abs(az - pz)) < (std::abs(bx - px) + std::abs(bz - pz));
            });

            for (const auto* chunk_ptr : chunks) {
                const Chunk& chunk = *chunk_ptr;
                combined_vertices.insert(combined_vertices.end(), chunk.vertices.begin(), chunk.vertices.end());
                for (uint32_t idx : chunk.indices) {
                    combined_indices.push_back(idx + vertex_offset);
                }
                vertex_offset += static_cast<uint32_t>(chunk.vertices.size());
            }

            all_vertices_size = combined_vertices.size();
            all_indices_size = combined_indices.size();

            pending_vertices = std::move(combined_vertices);
            pending_indices = std::move(combined_indices);
            mesh_ready.store(true);

            if (enable_validation_layers) {
                std::cerr << "\033[90m[Verbose] Generated mesh with "
                    << all_vertices_size << " vertices, "
                    << all_indices_size << " indices, "
                    << total_generated << " chunks, "
                    << chunk_map.size() << " total loaded chunks\n";
            }

            generating_mesh.store(false);
        }

        void create_chunk_mesh(Chunk& chunk) {
            // Pre-calculate chunk offset
            float ox = chunk.x * 16.0f;
            float oz = chunk.z * 16.0f;

            std::unordered_map<Vertex, uint32_t> vert_map;

            for (int lx = 0; lx < 16; lx++) {
                for (int ly = 0; ly < 383; ly++) {
                    for (int lz = 0; lz < 16; lz++) {
                        BlockType block_type = chunk.blocks[lx][ly][lz];
                        if (block_type == BlockType::AIR || block_type == BlockType::WATER) {
                            continue;
                        }

                        float wx = ox + lx;
                        float wy = ly;
                        float wz = oz + lz;

                        if (should_render_block_face(chunk, lx, ly, lz, BlockFace::TOP)) add_face_top(chunk, wx, wy, wz, block_type, vert_map);
                        if (should_render_block_face(chunk, lx, ly, lz, BlockFace::BOTTOM)) add_face_bottom(chunk, wx, wy, wz, block_type, vert_map);
                        if (should_render_block_face(chunk, lx, ly, lz, BlockFace::NORTH)) add_face_north(chunk, wx, wy, wz, block_type, vert_map);
                        if (should_render_block_face(chunk, lx, ly, lz, BlockFace::SOUTH)) add_face_south(chunk, wx, wy, wz, block_type, vert_map);
                        if (should_render_block_face(chunk, lx, ly, lz, BlockFace::EAST)) add_face_east(chunk, wx, wy, wz, block_type, vert_map);
                        if (should_render_block_face(chunk, lx, ly, lz, BlockFace::WEST)) add_face_west(chunk, wx, wy, wz, block_type, vert_map);
                    }
                }
            }
        }

        uint32_t get_vertex_index(Chunk& chunk, const Vertex& vertex, std::unordered_map<Vertex, uint32_t>& vert_map) {
            auto it = vert_map.find(vertex);
            if (it == vert_map.end()) {
                uint32_t index = static_cast<uint32_t>(chunk.vertices.size());
                chunk.vertices.push_back(vertex);
                vert_map[vertex] = index;
                return index;
            }
            return it->second;
        }

        void add_face_top(Chunk& chunk, float x, float y, float z, BlockType block_type, std::unordered_map<Vertex, uint32_t>& vert_map) {
            float tex_id = Block::get_texture_index(block_type);
            glm::vec3 color = (block_type == BlockType::GRASS || block_type == BlockType::LEAVES) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f);

            Vertex v0 = { {x, y + 1, z + 1}, color, {0.0f, 1.0f}, tex_id };
            Vertex v1 = { {x + 1, y + 1, z + 1}, color, {1.0f, 1.0f}, tex_id };
            Vertex v2 = { {x + 1, y + 1, z}, color, {1.0f, 0.0f}, tex_id };
            Vertex v3 = { {x, y + 1, z}, color, {0.0f, 0.0f}, tex_id };

            uint32_t i0 = get_vertex_index(chunk, v0, vert_map);
            uint32_t i1 = get_vertex_index(chunk, v1, vert_map);
            uint32_t i2 = get_vertex_index(chunk, v2, vert_map);
            uint32_t i3 = get_vertex_index(chunk, v3, vert_map);

            chunk.indices.insert(chunk.indices.end(), { i0, i1, i2, i0, i2, i3 });
        }

        void add_face_bottom(Chunk& chunk, float x, float y, float z, BlockType block_type, std::unordered_map<Vertex, uint32_t>& vert_map) {
            float tex_id = Block::get_texture_index(block_type == BlockType::GRASS ? BlockType::DIRT : block_type);
            glm::vec3 color = block_type == BlockType::LEAVES ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f);

            Vertex v0 = { {x, y, z}, color, {0.0f, 0.0f}, tex_id };
            Vertex v1 = { {x + 1, y, z}, color, {1.0f, 0.0f}, tex_id };
            Vertex v2 = { {x + 1, y, z + 1}, color, {1.0f, 1.0f}, tex_id };
            Vertex v3 = { {x, y, z + 1}, color, {0.0f, 1.0f}, tex_id };

            uint32_t i0 = get_vertex_index(chunk, v0, vert_map);
            uint32_t i1 = get_vertex_index(chunk, v1, vert_map);
            uint32_t i2 = get_vertex_index(chunk, v2, vert_map);
            uint32_t i3 = get_vertex_index(chunk, v3, vert_map);

            chunk.indices.insert(chunk.indices.end(), { i0, i1, i2, i0, i2, i3 });
        }

        void add_face_north(Chunk& chunk, float x, float y, float z, BlockType block_type, std::unordered_map<Vertex, uint32_t>& vert_map) {
            float tex_id = Block::get_texture_index(block_type == BlockType::GRASS ? BlockType::DIRT : block_type);
            glm::vec3 color = block_type == BlockType::LEAVES ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f);

            Vertex v0 = { {x, y, z + 1}, color, {0.0f, 0.0f}, tex_id };
            Vertex v1 = { {x + 1, y, z + 1}, color, {1.0f, 0.0f}, tex_id };
            Vertex v2 = { {x + 1, y + 1, z + 1}, color, {1.0f, 1.0f}, tex_id };
            Vertex v3 = { {x, y + 1, z + 1}, color, {0.0f, 1.0f}, tex_id };

            uint32_t i0 = get_vertex_index(chunk, v0, vert_map);
            uint32_t i1 = get_vertex_index(chunk, v1, vert_map);
            uint32_t i2 = get_vertex_index(chunk, v2, vert_map);
            uint32_t i3 = get_vertex_index(chunk, v3, vert_map);

            chunk.indices.insert(chunk.indices.end(), { i0, i1, i2, i0, i2, i3 });
        }

        void add_face_south(Chunk& chunk, float x, float y, float z, BlockType block_type, std::unordered_map<Vertex, uint32_t>& vert_map) {
            float tex_id = Block::get_texture_index(block_type == BlockType::GRASS ? BlockType::DIRT : block_type);
            glm::vec3 color = block_type == BlockType::LEAVES ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f);

            Vertex v0 = { {x + 1, y, z}, color, {0.0f, 0.0f}, tex_id };
            Vertex v1 = { {x, y, z}, color, {1.0f, 0.0f}, tex_id };
            Vertex v2 = { {x, y + 1, z}, color, {1.0f, 1.0f}, tex_id };
            Vertex v3 = { {x + 1, y + 1, z}, color, {0.0f, 1.0f}, tex_id };

            uint32_t i0 = get_vertex_index(chunk, v0, vert_map);
            uint32_t i1 = get_vertex_index(chunk, v1, vert_map);
            uint32_t i2 = get_vertex_index(chunk, v2, vert_map);
            uint32_t i3 = get_vertex_index(chunk, v3, vert_map);

            chunk.indices.insert(chunk.indices.end(), { i0, i1, i2, i0, i2, i3 });
        }

        void add_face_east(Chunk& chunk, float x, float y, float z, BlockType block_type, std::unordered_map<Vertex, uint32_t>& vert_map) {
            float tex_id = Block::get_texture_index(block_type == BlockType::GRASS ? BlockType::DIRT : block_type);
            glm::vec3 color = block_type == BlockType::LEAVES ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f);

            Vertex v0 = { {x + 1, y, z + 1}, color, {0.0f, 0.0f}, tex_id };
            Vertex v1 = { {x + 1, y, z}, color, {1.0f, 0.0f}, tex_id };
            Vertex v2 = { {x + 1, y + 1, z}, color, {1.0f, 1.0f}, tex_id };
            Vertex v3 = { {x + 1, y + 1, z + 1}, color, {0.0f, 1.0f}, tex_id };

            uint32_t i0 = get_vertex_index(chunk, v0, vert_map);
            uint32_t i1 = get_vertex_index(chunk, v1, vert_map);
            uint32_t i2 = get_vertex_index(chunk, v2, vert_map);
            uint32_t i3 = get_vertex_index(chunk, v3, vert_map);

            chunk.indices.insert(chunk.indices.end(), { i0, i1, i2, i0, i2, i3 });
        }

        void add_face_west(Chunk& chunk, float x, float y, float z, BlockType block_type, std::unordered_map<Vertex, uint32_t>& vert_map) {
            float tex_id = Block::get_texture_index(block_type == BlockType::GRASS ? BlockType::DIRT : block_type);
            glm::vec3 color = block_type == BlockType::LEAVES ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f);

            Vertex v0 = { {x, y, z}, color, {0.0f, 0.0f}, tex_id };
            Vertex v1 = { {x, y, z + 1}, color, {1.0f, 0.0f}, tex_id };
            Vertex v2 = { {x, y + 1, z + 1}, color, {1.0f, 1.0f}, tex_id };
            Vertex v3 = { {x, y + 1, z}, color, {0.0f, 1.0f}, tex_id };

            uint32_t i0 = get_vertex_index(chunk, v0, vert_map);
            uint32_t i1 = get_vertex_index(chunk, v1, vert_map);
            uint32_t i2 = get_vertex_index(chunk, v2, vert_map);
            uint32_t i3 = get_vertex_index(chunk, v3, vert_map);

            chunk.indices.insert(chunk.indices.end(), { i0, i1, i2, i0, i2, i3 });
        }

        enum class BlockFace {
            TOP, BOTTOM, NORTH, SOUTH, EAST, WEST
        };

        bool should_render_block_face(const Chunk& chunk, int x, int y, int z, BlockFace face) {
            int nx = x, ny = y, nz = z;
            switch (face) {
            case BlockFace::TOP: ny++; break;
            case BlockFace::BOTTOM: ny--; break;
            case BlockFace::NORTH: nz++; break;
            case BlockFace::SOUTH: nz--; break;
            case BlockFace::EAST: nx++; break;
            case BlockFace::WEST: nx--; break;
            }
            if (nx < 0 or nx >= 16 or nz < 0 or nz >= 16 or ny < 0 or ny >= 383) {
                return true;
            }
            BlockType neighbor = chunk.blocks[nx][ny][nz];
            return (neighbor == BlockType::AIR or neighbor == BlockType::WATER or neighbor == BlockType::LEAVES);
        }

        void update_generate_world() {
            while (!glfwWindowShouldClose(window)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
                static int last_chunk_x = INT_MAX;
                static int last_chunk_z = INT_MAX;

                int current_chunk_x = static_cast<int>(camera_pos.x) / 16;
                int current_chunk_z = static_cast<int>(camera_pos.z) / 16;
                if (current_chunk_x != last_chunk_x or current_chunk_z != last_chunk_z) {
                    generate_world_mesh();
                    last_chunk_x = current_chunk_x;
                    last_chunk_z = current_chunk_z;
                }
            }
        }

        void update_process_input() {
            while (!glfwWindowShouldClose(window)) {
                // delta time
                float current_frame = glfwGetTime();
                delta_time = current_frame - last_frame;
                last_frame = current_frame;
                process_input();
            }
        }

        void main_loop() {
            std::thread mesh_thread(&CraftbuildGraphics::update_generate_world, this);
            std::thread input_thread(&CraftbuildGraphics::update_process_input, this);
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();

                if (mesh_ready.load()) {
                    update_buffers();
                }

                draw_frame();
            }
            if (mesh_thread.joinable()) mesh_thread.join();
            if (input_thread.joinable()) input_thread.join();
        }

        void cleanup_swap_chain() {
            vkDestroyImageView(device, depth_image_view, nullptr);
            vkDestroyImage(device, depth_image, nullptr);
            vkFreeMemory(device, depth_image_memory, nullptr);
            vkDestroyImageView(device, color_image_view, nullptr);
            vkDestroyImage(device, color_image, nullptr);
            vkFreeMemory(device, color_image_memory, nullptr);
            for (auto framebuffer : swap_chain_framebuffers) {
                vkDestroyFramebuffer(device, framebuffer, nullptr);
            }
            for (auto image_view : swap_chain_image_views) {
                vkDestroyImageView(device, image_view, nullptr);
            }
            vkDestroySwapchainKHR(device, swap_chain, nullptr);
        }

        void recreate_swap_chain() {
            int width = 0, height = 0;
            glfwGetFramebufferSize(window, &width, &height);
            while (width == 0 or height == 0) {
                glfwGetFramebufferSize(window, &width, &height);
                glfwWaitEvents();
            }

            vkDeviceWaitIdle(device);

            cleanup_swap_chain();

            create_swap_chain();
            create_image_views();
            create_render_pass();
            create_graphics_pipeline();
            create_color_resources();
            create_depth_resources();
            create_framebuffers();
            create_command_buffers();
        }

        void create_instance() {
            if (enable_validation_layers and !check_validation_layer_support()) {
                throw std::runtime_error("Validation layers requested, but not available!");
            }

            VkApplicationInfo app_info{};
            app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            app_info.pApplicationName = "Craftbuild";
            app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            app_info.pEngineName = "No Engine";
            app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            app_info.apiVersion = VK_API_VERSION_1_0;

            VkInstanceCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            create_info.pApplicationInfo = &app_info;

            auto extensions = get_required_extensions();
            create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            create_info.ppEnabledExtensionNames = extensions.data();

            VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
            if (enable_validation_layers) {
                create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
                create_info.ppEnabledLayerNames = validation_layers.data();

                populate_debug_messenger_create_info(debug_create_info);
                create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
            }
            else {
                create_info.enabledLayerCount = 0;
                create_info.pNext = nullptr;
            }

            if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create instance!");
            }
        }

        void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info) {
            create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            create_info.pfnUserCallback = debug_callback;
            create_info.pUserData = this;
        }

        void setup_debug_messenger() {
            if (!enable_validation_layers) return;

            VkDebugUtilsMessengerCreateInfoEXT create_info;
            populate_debug_messenger_create_info(create_info);

            if (create_debug_utils_messenger_ext(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
                throw std::runtime_error("Failed to set up debug messenger!");
            }
        }

        void create_surface() {
            if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create window surface!");
            }
        }

        void pick_physical_device() {
            uint32_t device_count = 0;
            vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

            if (device_count == 0) {
                throw std::runtime_error("Failed to find GPUs with Vulkan support!");
            }

            std::vector<VkPhysicalDevice> devices(device_count);
            vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

            for (const auto& device : devices) {
                if (is_device_suitable(device)) {
                    physical_device = device;
                    msaa_samples = get_max_usable_sample_count();
                    break;
                }
            }

            if (physical_device == VK_NULL_HANDLE) {
                throw std::runtime_error("Failed to find a suitable GPU!");
            }
        }

        VkSampleCountFlagBits get_max_usable_sample_count() {
            VkPhysicalDeviceProperties physical_device_properties;
            vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);

            VkSampleCountFlags counts = physical_device_properties.limits.framebufferColorSampleCounts & physical_device_properties.limits.framebufferDepthSampleCounts;
            if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
            if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
            if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
            if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
            if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
            if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

            return VK_SAMPLE_COUNT_1_BIT;
        }

        void create_logical_device() {
            QueueFamilyIndices indices = find_queue_families(physical_device);

            std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
            std::set<uint32_t> unique_queue_families = { indices.graphics_family.value(), indices.present_family.value() };

            float queue_priority = 1.0f;
            for (uint32_t queue_family : unique_queue_families) {
                VkDeviceQueueCreateInfo queue_create_info{};
                queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_info.queueFamilyIndex = queue_family;
                queue_create_info.queueCount = 1;
                queue_create_info.pQueuePriorities = &queue_priority;
                queue_create_infos.push_back(queue_create_info);
            }

            VkPhysicalDeviceFeatures device_features{};
            device_features.samplerAnisotropy = VK_TRUE;
            device_features.sampleRateShading = VK_TRUE;

            VkDeviceCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

            create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
            create_info.pQueueCreateInfos = queue_create_infos.data();

            create_info.pEnabledFeatures = &device_features;

            create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
            create_info.ppEnabledExtensionNames = device_extensions.data();

            if (enable_validation_layers) {
                create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
                create_info.ppEnabledLayerNames = validation_layers.data();
            }
            else {
                create_info.enabledLayerCount = 0;
            }

            if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create logical device!");
            }

            vkGetDeviceQueue(device, indices.graphics_family.value(), 0, &graphics_queue);
            vkGetDeviceQueue(device, indices.present_family.value(), 0, &present_queue);
        }

        void create_swap_chain() {
            SwapChainSupportDetails swap_chain_support = query_swap_chain_support(physical_device);

            VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swap_chain_support.formats);
            VkPresentModeKHR present_mode = choose_swap_present_mode(swap_chain_support.present_modes);
            VkExtent2D extent = choose_swap_extent(swap_chain_support.capabilities);

            uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
            if (swap_chain_support.capabilities.maxImageCount > 0 and image_count > swap_chain_support.capabilities.maxImageCount) {
                image_count = swap_chain_support.capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            create_info.surface = surface;

            create_info.minImageCount = image_count;
            create_info.imageFormat = surface_format.format;
            create_info.imageColorSpace = surface_format.colorSpace;
            create_info.imageExtent = extent;
            create_info.imageArrayLayers = 1;
            create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            QueueFamilyIndices indices = find_queue_families(physical_device);
            uint32_t queue_family_indices[] = { indices.graphics_family.value(), indices.present_family.value() };

            if (indices.graphics_family != indices.present_family) {
                create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                create_info.queueFamilyIndexCount = 2;
                create_info.pQueueFamilyIndices = queue_family_indices;
            }
            else {
                create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            }

            create_info.preTransform = swap_chain_support.capabilities.currentTransform;
            create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            create_info.presentMode = present_mode;
            create_info.clipped = VK_TRUE;

            if (vkCreateSwapchainKHR(device, &create_info, nullptr, &swap_chain) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create swap chain!");
            }

            vkGetSwapchainImagesKHR(device, swap_chain, &image_count, nullptr);
            swap_chain_images.resize(image_count);
            vkGetSwapchainImagesKHR(device, swap_chain, &image_count, swap_chain_images.data());

            swap_chain_image_format = surface_format.format;
            swap_chain_extent = extent;
        }

        void create_image_views() {
            swap_chain_image_views.resize(swap_chain_images.size());

            for (uint32_t i = 0; i < swap_chain_images.size(); i++) {
                swap_chain_image_views[i] = create_image_view(swap_chain_images[i], swap_chain_image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
            }
        }

        void create_render_pass() {
            VkAttachmentDescription color_attachment{};
            color_attachment.format = swap_chain_image_format;
            color_attachment.samples = msaa_samples;
            color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentReference color_attachment_ref{};
            color_attachment_ref.attachment = 0;
            color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentDescription depth_attachment{};
            depth_attachment.format = find_depth_format();
            depth_attachment.samples = msaa_samples;
            depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depth_attachment_ref{};
            depth_attachment_ref.attachment = 1;
            depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentDescription color_attachment_resolve{};
            color_attachment_resolve.format = swap_chain_image_format;
            color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
            color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            color_attachment_resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference color_attachment_resolve_ref{};
            color_attachment_resolve_ref.attachment = 2;
            color_attachment_resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &color_attachment_ref;
            subpass.pDepthStencilAttachment = &depth_attachment_ref;
            subpass.pResolveAttachments = &color_attachment_resolve_ref;

            VkSubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            std::array<VkAttachmentDescription, 3> attachments = { color_attachment, depth_attachment, color_attachment_resolve };
            VkRenderPassCreateInfo render_pass_info{};
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
            render_pass_info.pAttachments = attachments.data();
            render_pass_info.subpassCount = 1;
            render_pass_info.pSubpasses = &subpass;
            render_pass_info.dependencyCount = 1;
            render_pass_info.pDependencies = &dependency;

            if (vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create render pass!");
            }
        }

        void create_descriptor_set_layout() {
            VkDescriptorSetLayoutBinding ubo_layout_binding{};
            ubo_layout_binding.binding = 0;
            ubo_layout_binding.descriptorCount = 1;
            ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            ubo_layout_binding.pImmutableSamplers = nullptr;
            ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            VkDescriptorSetLayoutBinding sampler_layout_binding{};
            sampler_layout_binding.binding = 1;
            sampler_layout_binding.descriptorCount = 12;
            sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            sampler_layout_binding.pImmutableSamplers = nullptr;
            sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            std::array<VkDescriptorSetLayoutBinding, 2> bindings = { ubo_layout_binding, sampler_layout_binding };
            VkDescriptorSetLayoutCreateInfo layout_info{};
            layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
            layout_info.pBindings = bindings.data();

            if (vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &descriptor_set_layout) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create descriptor set layout!");
            }
        }

        void create_graphics_pipeline() {
            auto vert_shader_code = read_file("Game/Craftbuild/Shader/Cache/vert.spv");
            auto frag_shader_code = read_file("Game/Craftbuild/Shader/Cache/frag.spv");

            VkShaderModule vert_shader_module = create_shader_module(vert_shader_code);
            VkShaderModule frag_shader_module = create_shader_module(frag_shader_code);

            VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
            vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vert_shader_stage_info.module = vert_shader_module;
            vert_shader_stage_info.pName = "main";

            VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
            frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            frag_shader_stage_info.module = frag_shader_module;
            frag_shader_stage_info.pName = "main";

            VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_info, frag_shader_stage_info };

            VkPipelineVertexInputStateCreateInfo vertex_input_info{};
            vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

            auto binding_description = Vertex::get_binding_description();
            auto attribute_descriptions = Vertex::get_attribute_descriptions();

            vertex_input_info.vertexBindingDescriptionCount = 1;
            vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
            vertex_input_info.pVertexBindingDescriptions = &binding_description;
            vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

            VkPipelineInputAssemblyStateCreateInfo input_assembly{};
            input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            input_assembly.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo viewport_state{};
            viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state.viewportCount = 1;
            viewport_state.scissorCount = 1;

            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_TRUE;
            multisampling.minSampleShading = 0.2f;
            multisampling.rasterizationSamples = msaa_samples;

            VkPipelineColorBlendAttachmentState color_blend_attachment{};
            color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            color_blend_attachment.blendEnable = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo color_blending{};
            color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blending.logicOpEnable = VK_FALSE;
            color_blending.attachmentCount = 1;
            color_blending.pAttachments = &color_blend_attachment;

            VkPipelineDepthStencilStateCreateInfo depth_stencil{};
            depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil.depthTestEnable = VK_TRUE;
            depth_stencil.depthWriteEnable = VK_TRUE;
            depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
            depth_stencil.depthBoundsTestEnable = VK_FALSE;
            depth_stencil.stencilTestEnable = VK_FALSE;

            std::vector<VkDynamicState> dynamic_states = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };

            VkPipelineDynamicStateCreateInfo dynamic_state{};
            dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
            dynamic_state.pDynamicStates = dynamic_states.data();

            VkPipelineLayoutCreateInfo pipeline_layout_info{};
            pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_info.setLayoutCount = 1;
            pipeline_layout_info.pSetLayouts = &descriptor_set_layout;

            if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create pipeline layout!");
            }

            VkGraphicsPipelineCreateInfo pipeline_info{};
            pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_info.stageCount = 2;
            pipeline_info.pStages = shader_stages;
            pipeline_info.pVertexInputState = &vertex_input_info;
            pipeline_info.pInputAssemblyState = &input_assembly;
            pipeline_info.pViewportState = &viewport_state;
            pipeline_info.pRasterizationState = &rasterizer;
            pipeline_info.pMultisampleState = &multisampling;
            pipeline_info.pColorBlendState = &color_blending;
            pipeline_info.pDepthStencilState = &depth_stencil;
            pipeline_info.pDynamicState = &dynamic_state;
            pipeline_info.layout = pipeline_layout;
            pipeline_info.renderPass = render_pass;
            pipeline_info.subpass = 0;
            pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

            if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create graphics pipeline!");
            }

            vkDestroyShaderModule(device, frag_shader_module, nullptr);
            vkDestroyShaderModule(device, vert_shader_module, nullptr);
        }

        void create_framebuffers() {
            swap_chain_framebuffers.resize(swap_chain_image_views.size());

            for (size_t i = 0; i < swap_chain_image_views.size(); i++) {
                std::array<VkImageView, 3> attachments = { color_image_view, depth_image_view, swap_chain_image_views[i] };

                VkFramebufferCreateInfo framebuffer_info{};
                framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebuffer_info.renderPass = render_pass;
                framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
                framebuffer_info.pAttachments = attachments.data();
                framebuffer_info.width = swap_chain_extent.width;
                framebuffer_info.height = swap_chain_extent.height;
                framebuffer_info.layers = 1;

                if (vkCreateFramebuffer(device, &framebuffer_info, nullptr, &swap_chain_framebuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create framebuffer!");
                }
            }
        }

        void create_command_pool() {
            QueueFamilyIndices queue_family_indices = find_queue_families(physical_device);

            VkCommandPoolCreateInfo pool_info{};
            pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();

            if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pool) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create graphics command pool!");
            }
        }

        void create_color_resources() {
            VkFormat color_format = swap_chain_image_format;

            create_image(swap_chain_extent.width, swap_chain_extent.height, 1, msaa_samples, color_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, color_image, color_image_memory);
            color_image_view = create_image_view(color_image, color_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        }

        void create_depth_resources() {
            VkFormat depth_format = find_depth_format();

            create_image(swap_chain_extent.width, swap_chain_extent.height, 1, msaa_samples, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_image, depth_image_memory);
            depth_image_view = create_image_view(depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
        }

        VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
            for (VkFormat format : candidates) {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

                if (tiling == VK_IMAGE_TILING_LINEAR and (props.linearTilingFeatures & features) == features) {
                    return format;
                }
                else if (tiling == VK_IMAGE_TILING_OPTIMAL and (props.optimalTilingFeatures & features) == features) {
                    return format;
                }
            }

            throw std::runtime_error("Failed to find supported format!");
        }

        VkFormat find_depth_format() {
            return find_supported_format(
                { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
            );
        }

        bool has_stencil_component(VkFormat format) {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT or format == VK_FORMAT_D24_UNORM_S8_UINT;
        }

        void create_texture_image(BlockType type) {
            int tex_width, tex_height, tex_channels;
            stbi_uc* pixels = stbi_load(Block::get_texture(type), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
            VkDeviceSize image_size = tex_width * tex_height * 4;
            mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1;

            if (!pixels) {
                throw std::runtime_error("Failed to load texture image!");
            }

            VkBuffer staging_buffer;
            VkDeviceMemory staging_buffer_memory;
            create_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

            void* data;
            vkMapMemory(device, staging_buffer_memory, 0, image_size, 0, &data);
            memcpy(data, pixels, static_cast<size_t>(image_size));
            vkUnmapMemory(device, staging_buffer_memory);

            stbi_image_free(pixels);

            create_image(tex_width, tex_height, mip_levels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, block_textures[type], block_textures_memory[type]);

            transition_image_layout(block_textures[type], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels);
            copy_buffer_to_image(staging_buffer, block_textures[type], static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height));

            vkDestroyBuffer(device, staging_buffer, nullptr);
            vkFreeMemory(device, staging_buffer_memory, nullptr);

            generate_mipmaps(block_textures[type], VK_FORMAT_R8G8B8A8_SRGB, tex_width, tex_height, mip_levels);
        }

        void generate_mipmaps(VkImage image, VkFormat image_format, int32_t tex_width, int32_t tex_height, uint32_t mip_levels) {
            VkFormatProperties format_properties;
            vkGetPhysicalDeviceFormatProperties(physical_device, image_format, &format_properties);

            if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
                throw std::runtime_error("Texture image format does not support linear blitting!");
            }

            VkCommandBuffer command_buffer = begin_single_time_commands();

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = image;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;

            int32_t mip_width = tex_width;
            int32_t mip_height = tex_height;

            for (uint32_t i = 1; i < mip_levels; i++) {
                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

                vkCmdPipelineBarrier(command_buffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);

                VkImageBlit blit{};
                blit.srcOffsets[0] = { 0, 0, 0 };
                blit.srcOffsets[1] = { mip_width, mip_height, 1 };
                blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount = 1;
                blit.dstOffsets[0] = { 0, 0, 0 };
                blit.dstOffsets[1] = { mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
                blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount = 1;

                vkCmdBlitImage(command_buffer,
                    image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &blit,
                    VK_FILTER_LINEAR);

                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(command_buffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);

                if (mip_width > 1) mip_width /= 2;
                if (mip_height > 1) mip_height /= 2;
            }

            barrier.subresourceRange.baseMipLevel = mip_levels - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(command_buffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            end_single_time_commands(command_buffer);
        }

        uint32_t mip_levels;

        void create_image(uint32_t width, uint32_t height, uint32_t mip_levels, VkSampleCountFlagBits num_samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory) {
            VkImageCreateInfo image_info{};
            image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_info.imageType = VK_IMAGE_TYPE_2D;
            image_info.extent.width = width;
            image_info.extent.height = height;
            image_info.extent.depth = 1;
            image_info.mipLevels = mip_levels;
            image_info.arrayLayers = 1;
            image_info.format = format;
            image_info.tiling = tiling;
            image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            image_info.usage = usage;
            image_info.samples = num_samples;
            image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateImage(device, &image_info, nullptr, &image) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create image!");
            }

            VkMemoryRequirements mem_requirements;
            vkGetImageMemoryRequirements(device, image, &mem_requirements);

            VkMemoryAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = mem_requirements.size;
            alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);

            if (vkAllocateMemory(device, &alloc_info, nullptr, &image_memory) != VK_SUCCESS) {
                throw std::runtime_error("Failed to allocate image memory!");
            }

            vkBindImageMemory(device, image, image_memory, 0);
        }

        void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels) {
            VkCommandBuffer command_buffer = begin_single_time_commands();

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = old_layout;
            barrier.newLayout = new_layout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = mip_levels;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            VkPipelineStageFlags source_stage;
            VkPipelineStageFlags destination_stage;

            if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED and new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL and new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else {
                throw std::invalid_argument("Unsupported layout transition!");
            }

            vkCmdPipelineBarrier(
                command_buffer,
                source_stage, destination_stage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            end_single_time_commands(command_buffer);
        }

        void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
            VkCommandBuffer command_buffer = begin_single_time_commands();

            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = {
                width,
                height,
                1
            };

            vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            end_single_time_commands(command_buffer);
        }

        void load_all_textures() {
            std::vector<BlockType> block_types = {
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
                BlockType::GRASS_PLANT
            };

            for (auto type : block_types) {
                create_texture_image(type);
                block_texture_views[type] = create_image_view(block_textures[type], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mip_levels);
            }
        }

        VkImageView create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels) {
            VkImageViewCreateInfo view_info{};
            view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_info.image = image;
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_info.format = format;
            view_info.subresourceRange.aspectMask = aspect_flags;
            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = mip_levels;
            view_info.subresourceRange.baseArrayLayer = 0;
            view_info.subresourceRange.layerCount = 1;

            VkImageView image_view;
            if (vkCreateImageView(device, &view_info, nullptr, &image_view) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create texture image view!");
            }

            return image_view;
        }

        void create_texture_sampler() {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(physical_device, &properties);

            VkSamplerCreateInfo sampler_info{};
            sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            sampler_info.magFilter = VK_FILTER_NEAREST;
            sampler_info.minFilter = VK_FILTER_NEAREST;
            sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_info.anisotropyEnable = VK_TRUE;
            sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
            sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            sampler_info.unnormalizedCoordinates = VK_FALSE;
            sampler_info.compareEnable = VK_FALSE;
            sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
            sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            sampler_info.minLod = 0.0f;
            sampler_info.maxLod = static_cast<float>(mip_levels);
            sampler_info.mipLodBias = 0.0f;

            if (vkCreateSampler(device, &sampler_info, nullptr, &texture_sampler) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create texture sampler!");
            }
        }

        void create_vertex_buffer(const std::vector<Vertex>& vertices) {
            VkDeviceSize buffer_size = sizeof(Vertex) * vertices.size();

            VkBuffer staging_buffer;
            VkDeviceMemory staging_buffer_memory;
            create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

            void* data;
            vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
            memcpy(data, vertices.data(), buffer_size);
            vkUnmapMemory(device, staging_buffer_memory);

            create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer, vertex_buffer_memory);

            copy_buffer(staging_buffer, vertex_buffer, buffer_size);

            vkDestroyBuffer(device, staging_buffer, nullptr);
            vkFreeMemory(device, staging_buffer_memory, nullptr);
        }

        void create_index_buffer(const std::vector<uint32_t>& indices) {
            VkDeviceSize buffer_size = sizeof(uint32_t) * indices.size();

            VkBuffer staging_buffer;
            VkDeviceMemory staging_buffer_memory;
            create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

            void* data;
            vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
            memcpy(data, indices.data(), buffer_size);
            vkUnmapMemory(device, staging_buffer_memory);

            create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer, index_buffer_memory);

            copy_buffer(staging_buffer, index_buffer, buffer_size);

            vkDestroyBuffer(device, staging_buffer, nullptr);
            vkFreeMemory(device, staging_buffer_memory, nullptr);
        }

        void create_uniform_buffers() {
            VkDeviceSize buffer_size = sizeof(UniformBufferObject);

            uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);
            uniform_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);
            uniform_buffers_mapped.resize(MAX_FRAMES_IN_FLIGHT);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                create_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniform_buffers[i], uniform_buffers_memory[i]);

                vkMapMemory(device, uniform_buffers_memory[i], 0, buffer_size, 0, &uniform_buffers_mapped[i]);
            }
        }

        void create_descriptor_pool() {
            std::array<VkDescriptorPoolSize, 2> pool_sizes{};
            pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            pool_sizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            pool_sizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 12;
            VkDescriptorPoolCreateInfo pool_info{};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
            pool_info.pPoolSizes = pool_sizes.data();
            pool_info.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            if (vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create descriptor pool!");
            }
        }

        void create_descriptor_sets() {
            std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptor_set_layout);
            VkDescriptorSetAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            alloc_info.descriptorPool = descriptor_pool;
            alloc_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            alloc_info.pSetLayouts = layouts.data();
            descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);
            if (vkAllocateDescriptorSets(device, &alloc_info, descriptor_sets.data()) != VK_SUCCESS) {
                throw std::runtime_error("Failed to allocate descriptor sets!");
            }
            std::vector<VkDescriptorImageInfo> image_infos;
            std::vector<BlockType> block_types = {
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
                BlockType::GRASS_PLANT
            };
            for (auto type : block_types) {
                VkDescriptorImageInfo image_info{};
                image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                image_info.imageView = block_texture_views[type];
                image_info.sampler = texture_sampler;
                image_infos.push_back(image_info);
            }
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                VkDescriptorBufferInfo buffer_info{};
                buffer_info.buffer = uniform_buffers[i];
                buffer_info.offset = 0;
                buffer_info.range = sizeof(UniformBufferObject);
                std::vector<VkWriteDescriptorSet> descriptor_writes;
                VkWriteDescriptorSet ubo_write{};
                ubo_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                ubo_write.dstSet = descriptor_sets[i];
                ubo_write.dstBinding = 0;
                ubo_write.dstArrayElement = 0;
                ubo_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                ubo_write.descriptorCount = 1;
                ubo_write.pBufferInfo = &buffer_info;
                descriptor_writes.push_back(ubo_write);
                VkWriteDescriptorSet sampler_write{};
                sampler_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                sampler_write.dstSet = descriptor_sets[i];
                sampler_write.dstBinding = 1;
                sampler_write.dstArrayElement = 0;
                sampler_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                sampler_write.descriptorCount = static_cast<uint32_t>(image_infos.size());
                sampler_write.pImageInfo = image_infos.data();
                descriptor_writes.push_back(sampler_write);
                vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptor_writes.size()),
                    descriptor_writes.data(), 0, nullptr);
            }
        }

        void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory) {
            VkBufferCreateInfo buffer_info{};
            buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_info.size = size;
            buffer_info.usage = usage;
            buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateBuffer(device, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create buffer!");
            }

            VkMemoryRequirements mem_requirements;
            vkGetBufferMemoryRequirements(device, buffer, &mem_requirements);

            VkMemoryAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = mem_requirements.size;
            alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);

            if (vkAllocateMemory(device, &alloc_info, nullptr, &buffer_memory) != VK_SUCCESS) {
                throw std::runtime_error("Failed to allocate buffer memory!");
            }

            vkBindBufferMemory(device, buffer, buffer_memory, 0);
        }

        VkCommandBuffer begin_single_time_commands() {
            VkCommandBufferAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc_info.commandPool = command_pool;
            alloc_info.commandBufferCount = 1;

            VkCommandBuffer command_buffer;
            vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);

            VkCommandBufferBeginInfo begin_info{};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(command_buffer, &begin_info);
            return command_buffer;
        }

        void end_single_time_commands(VkCommandBuffer command_buffer) {
            vkEndCommandBuffer(command_buffer);

            VkSubmitInfo submit_info{};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &command_buffer;

            vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
            vkQueueWaitIdle(graphics_queue);
            vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        }

        void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) {
            VkCommandBuffer command_buffer = begin_single_time_commands();
            VkBufferCopy copy_region{};
            copy_region.size = size;
            vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);
            end_single_time_commands(command_buffer);
        }

        uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) {
            VkPhysicalDeviceMemoryProperties mem_properties;
            vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);
            for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
                if ((type_filter & (1 << i)) and (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }
            throw std::runtime_error("Failed to find suitable memory type!");
        }

        void create_command_buffers() {
            command_buffers.resize(MAX_FRAMES_IN_FLIGHT);
            VkCommandBufferAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc_info.commandPool = command_pool;
            alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc_info.commandBufferCount = (uint32_t)command_buffers.size();
            if (vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data()) != VK_SUCCESS) {
                throw std::runtime_error("Failed to allocate command buffers!");
            }
        }

        void record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index) {
            VkCommandBufferBeginInfo begin_info{};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
                throw std::runtime_error("Failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo render_pass_info{};
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_info.renderPass = render_pass;
            render_pass_info.framebuffer = swap_chain_framebuffers[image_index];
            render_pass_info.renderArea.offset = { 0, 0 };
            render_pass_info.renderArea.extent = swap_chain_extent;

            std::array<VkClearValue, 2> clear_values{};
            clear_values[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clear_values[1].depthStencil = { 1.0f, 0 };
            render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
            render_pass_info.pClearValues = clear_values.data();

            vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)swap_chain_extent.width;
            viewport.height = (float)swap_chain_extent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            vkCmdSetViewport(command_buffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = swap_chain_extent;

            vkCmdSetScissor(command_buffer, 0, 1, &scissor);

            if (all_vertices_size and all_indices_size and vertex_buffer != VK_NULL_HANDLE and index_buffer != VK_NULL_HANDLE) {
                VkBuffer vertex_buffers[] = { vertex_buffer };
                VkDeviceSize offsets[] = { 0 };

                vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

                vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_sets[current_frame], 0, nullptr);

                vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(all_indices_size), 1, 0, 0, 0);
            }

            vkCmdEndRenderPass(command_buffer);

            if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
                throw std::runtime_error("Failed to record command buffer!");
            }
        }

        void create_sync_objects() {
            image_available_semaphores.resize(swap_chain_images.size());
            render_finished_semaphores.resize(swap_chain_images.size());
            in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
            images_in_flight.resize(swap_chain_images.size(), VK_NULL_HANDLE);

            VkSemaphoreCreateInfo semaphore_info{};
            semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkFenceCreateInfo fence_info{};
            fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            for (size_t i = 0; i < swap_chain_images.size(); i++) {
                if (vkCreateSemaphore(device, &semaphore_info, nullptr, &image_available_semaphores[i]) != VK_SUCCESS or
                    vkCreateSemaphore(device, &semaphore_info, nullptr, &render_finished_semaphores[i]) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create semaphores!");
                }
            }
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                if (vkCreateFence(device, &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create fences!");
                }
            }
        }

        void update_uniform_buffer(uint32_t current_image) {
            static auto start_time = std::chrono::high_resolution_clock::now();
            auto current_time = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
            UniformBufferObject ubo{};
            ubo.model = glm::mat4(1.0f);
            // Camera view matrix
            ubo.view = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
            // Projection matrix
            ubo.proj = glm::perspective(glm::radians(45.0f), swap_chain_extent.width / (float)swap_chain_extent.height, 0.1f, 500.0f);
            ubo.proj[1][1] *= -1;
            memcpy(uniform_buffers_mapped[current_image], &ubo, sizeof(ubo));
        }

        void update_descriptor_sets() {
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                VkDescriptorBufferInfo buffer_info{};
                buffer_info.buffer = uniform_buffers[i];
                buffer_info.offset = 0;
                buffer_info.range = sizeof(UniformBufferObject);
                std::vector<VkDescriptorImageInfo> image_infos;
                std::vector<BlockType> block_types = {
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
                    BlockType::GRASS_PLANT
                };
                for (auto type : block_types) {
                    VkDescriptorImageInfo image_info{};
                    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    image_info.imageView = block_texture_views[type];
                    image_info.sampler = texture_sampler;
                    image_infos.push_back(image_info);
                }
                std::array<VkWriteDescriptorSet, 2> descriptor_writes{};
                descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptor_writes[0].dstSet = descriptor_sets[i];
                descriptor_writes[0].dstBinding = 0;
                descriptor_writes[0].dstArrayElement = 0;
                descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptor_writes[0].descriptorCount = 1;
                descriptor_writes[0].pBufferInfo = &buffer_info;
                descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptor_writes[1].dstSet = descriptor_sets[i];
                descriptor_writes[1].dstBinding = 1;
                descriptor_writes[1].dstArrayElement = 0;
                descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptor_writes[1].descriptorCount = static_cast<uint32_t>(image_infos.size());
                descriptor_writes[1].pImageInfo = image_infos.data();
                vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
            }
        }

        void update_buffers() {
            std::vector<Vertex> all_vertices;
            std::vector<uint32_t> all_indices;
            {
                std::lock_guard<std::mutex> lock(mesh_mutex);
                if (!mesh_ready.load()) return;
                all_vertices.swap(pending_vertices);
                all_indices.swap(pending_indices);
                mesh_ready.store(false);
            }

            if (all_vertices.empty() or all_indices.empty()) {
                all_vertices_size = 0;
                all_indices_size = 0;
                if (enable_validation_layers) {
                    std::cerr << "\033[93m[Warning]\033[33m No vertices or indices to render, skipping buffer update.\n";
                }
                return;
            }

            vkDeviceWaitIdle(device);

            vkDestroyBuffer(device, vertex_buffer, nullptr);
            vkFreeMemory(device, vertex_buffer_memory, nullptr);
            vkDestroyBuffer(device, index_buffer, nullptr);
            vkFreeMemory(device, index_buffer_memory, nullptr);

            all_vertices_size = all_vertices.size();
            all_indices_size = all_indices.size();

            create_vertex_buffer(all_vertices);
            create_index_buffer(all_indices);

            if (!block_texture_views.empty()) {
                update_descriptor_sets();
            }

            recreate_command_buffers();
        }

        void recreate_command_buffers() {
            vkFreeCommandBuffers(device, command_pool, static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
            create_command_buffers();
        }

        void draw_frame() {
            vkWaitForFences(device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

            uint32_t image_index;
            VkResult result = vkAcquireNextImageKHR(device, swap_chain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                recreate_swap_chain();
                return;
            }
            else if (result != VK_SUCCESS and result != VK_SUBOPTIMAL_KHR) {
                throw std::runtime_error("Failed to acquire swap chain image!");
            }
            if (images_in_flight[image_index] != VK_NULL_HANDLE) {
                vkWaitForFences(device, 1, &images_in_flight[image_index], VK_TRUE, UINT64_MAX);
            }

            images_in_flight[image_index] = in_flight_fences[current_frame];

            update_uniform_buffer(current_frame);
            vkResetFences(device, 1, &in_flight_fences[current_frame]);
            vkResetCommandBuffer(command_buffers[current_frame], 0);
            record_command_buffer(command_buffers[current_frame], image_index);

            VkSubmitInfo submit_info{};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore wait_semaphores[] = { image_available_semaphores[current_frame] };
            VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = wait_semaphores;
            submit_info.pWaitDstStageMask = wait_stages;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &command_buffers[current_frame];

            VkSemaphore signal_semaphores[] = { render_finished_semaphores[image_index] };
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = signal_semaphores;

            if (vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to submit draw command buffer!");
            }

            VkPresentInfoKHR present_info{};
            present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            present_info.waitSemaphoreCount = 1;
            present_info.pWaitSemaphores = signal_semaphores;
            VkSwapchainKHR swap_chains[] = { swap_chain };

            present_info.swapchainCount = 1;
            present_info.pSwapchains = swap_chains;
            present_info.pImageIndices = &image_index;
            result = vkQueuePresentKHR(present_queue, &present_info);

            if (result == VK_ERROR_OUT_OF_DATE_KHR or result == VK_SUBOPTIMAL_KHR or framebuffer_resized) {
                framebuffer_resized = false;
                recreate_swap_chain();
            }
            else if (result != VK_SUCCESS) {
                throw std::runtime_error("Failed to present swap chain image!");
            }

            current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
        }

        VkShaderModule create_shader_module(const std::vector<char>& code) {
            VkShaderModuleCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            create_info.codeSize = code.size();
            create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

            VkShaderModule shader_module;
            if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create shader module!");
            }
            return shader_module;
        }

        VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) {
            for (const auto& available_format : available_formats) {
                if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB and available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return available_format;
                }
            }
            return available_formats[0];
        }

        VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) {
            for (const auto& available_present_mode : available_present_modes) {
                if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return available_present_mode;
                }
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                return capabilities.currentExtent;
            }
            else {
                int width, height;
                glfwGetFramebufferSize(window, &width, &height);
                VkExtent2D actual_extent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
                };
                actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
                return actual_extent;
            }
        }

        SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device) {
            SwapChainSupportDetails details;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
            uint32_t format_count;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
            if (format_count != 0) {
                details.formats.resize(format_count);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
            }
            uint32_t present_mode_count;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
            if (present_mode_count != 0) {
                details.present_modes.resize(present_mode_count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
            }
            return details;
        }

        bool is_device_suitable(VkPhysicalDevice device) {
            QueueFamilyIndices indices = find_queue_families(device);
            bool extensions_supported = check_device_extension_support(device);
            bool swap_chain_adequate = false;
            if (extensions_supported) {
                SwapChainSupportDetails swapChainSupport = query_swap_chain_support(device);
                swap_chain_adequate = !swapChainSupport.formats.empty() and !swapChainSupport.present_modes.empty();
            }
            VkPhysicalDeviceFeatures supported_features;
            vkGetPhysicalDeviceFeatures(device, &supported_features);
            return indices.is_complete() and extensions_supported and swap_chain_adequate and supported_features.samplerAnisotropy;
        }

        bool check_device_extension_support(VkPhysicalDevice device) {
            uint32_t extension_count;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
            std::vector<VkExtensionProperties> available_extensions(extension_count);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());
            std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());
            for (const auto& extension : available_extensions) {
                required_extensions.erase(extension.extensionName);
            }
            return required_extensions.empty();
        }

        QueueFamilyIndices find_queue_families(VkPhysicalDevice device) {
            QueueFamilyIndices indices;
            uint32_t queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
            std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());
            int i = 0;
            for (const auto& queue_family : queue_families) {
                if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphics_family = i;
                }
                VkBool32 present_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
                if (present_support) {
                    indices.present_family = i;
                }
                if (indices.is_complete()) {
                    break;
                }
                i++;
            }
            return indices;
        }

        std::vector<const char*> get_required_extensions() {
            uint32_t glfw_extension_count = 0;
            const char** glfw_extensions;
            glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
            std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
            if (enable_validation_layers) {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            return extensions;
        }

        bool check_validation_layer_support() {
            uint32_t layer_count;
            vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
            std::vector<VkLayerProperties> available_layers(layer_count);
            vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
            for (const char* layer_name : validation_layers) {
                bool layer_found = false;
                for (const auto& layer_properties : available_layers) {
                    if (strcmp(layer_name, layer_properties.layerName) == 0) {
                        layer_found = true;
                        break;
                    }
                }
                if (!layer_found) {
                    return false;
                }
            }
            return true;
        }

        static std::vector<char> read_file(const std::string& filename) {
            std::ifstream file(filename, std::ios::ate | std::ios::binary);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open file!");
            }
            size_t file_size = (size_t)file.tellg();
            std::vector<char> buffer(file_size);
            file.seekg(0);
            file.read(buffer.data(), file_size);
            file.close();
            return buffer;
        }

        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data) {
            CraftbuildGraphics* console = reinterpret_cast<CraftbuildGraphics*>(p_user_data);
            if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
                if (log_verbose) std::cerr << "\033[90m[Verbose] validation layer: " << p_callback_data->pMessage << "\n";
            }
            else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
                std::cerr << "\033[96m[Info] validation layer:\033[36m " << p_callback_data->pMessage << "\n";
            }
            else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
                std::cerr << "\033[93m[Warning] validation layer:\033[33m " << p_callback_data->pMessage << "\n";
            }
            else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
                std::cerr << "\033[91m[Error] validation layer:\033[31m " << p_callback_data->pMessage << "\n";
            }
            return VK_FALSE;
        }
    };
}
