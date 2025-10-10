#include "EditorApplication.h"

extern std::vector<BasePart *> g_instances;
extern lua_State *L_main;
extern Camera3D g_camera;

int main(int argc, char *argv[]) {
    EditorApplication app(argc, argv);
    return app.Run();
}