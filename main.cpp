#include <core.hpp>
#include <Graphics/Class/Main.hpp>

int main() {
#ifdef _DEBUG
    system("glslc Game/Craftbuild/Shader/shader.vert -o Game/Craftbuild/Shader/Cache/vert.spv");
    system("glslc Game/Craftbuild/Shader/shader.frag -o Game/Craftbuild/Shader/Cache/frag.spv");
#endif 

    try {
        Craftbuild::CraftbuildMain();
    }
    catch (const std::exception& e) {
        std::cerr << "\033[95m[Fatal]\033[35m " << e.what() << "\033[0m\n";
        return EXIT_FAILURE;
    }

    std::cerr << "\033[0m";

    return EXIT_SUCCESS;
}
