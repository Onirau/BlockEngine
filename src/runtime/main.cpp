#include "RuntimeApplication.h"

extern std::vector<BasePart*> g_instances;
extern lua_State* L_main;
extern Camera3D g_camera;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: BlockEngineRuntime <place_file>\n");
        return 1;
    }

    RuntimeApplication app(argv[1]);
    app.Run();
    return 0;
}