#include <cmath>
#include <cstdio>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <filesystem>

#include <lua.h>
#include <lualib.h>
#include <luacode.h>

#include "datatypes/Task.h"
#include "instances/BasePart.h"
#include "instances/Part.h"
#include "raylib.h"
#include "raymath.h"

#include "core/Renderer.h"
#include "core/LuaBindings.h"
#include "Global.h"

//Define global variables
std::vector<BasePart*> g_instances;
lua_State* L_main = nullptr;
Camera3D g_camera{};
#include "core/LuaBindings.h"

std::vector<BasePart*> g_instances;

#include "instances/DataModel.h"
#include "instances/Workspace.h"

lua_State* L_main = nullptr;

Camera3D g_camera{};

static float gYaw = 0.0f;
static float gPitch = 0.0f;

static Color backgroundColor{0, 0, 0, 0};

//Compile and run a Lua chunk from source in the current state.
//Lua signature: __RUN_CHUNK(source: string, chunkname: string?) -> (bool ok, string? err)
static int L_RunChunk(lua_State* L) {
    size_t len = 0;
    const char* src = luaL_checklstring(L, 1, &len);
    const char* name = luaL_optstring(L, 2, "ScriptChunk");

    size_t bcSize = 0;
    lua_CompileOptions opts{};
    opts.optimizationLevel = 1;
    opts.debugLevel = 1;
    const char* bytecode = luau_compile(src, len, &opts, &bcSize);

    int loadStatus = luau_load(L, name, bytecode, bcSize, 0);
    if (loadStatus != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        lua_pushboolean(L, 0);
        lua_pushstring(L, err ? err : "load error");
        return 2;
    }

    int status = lua_pcall(L, 0, 0, 0);
    if (status != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        lua_pushboolean(L, 0);
        lua_pushstring(L, err ? err : "runtime error");
        return 2;
    }

    lua_pushboolean(L, 1);
    lua_pushnil(L);
    return 2;
}

std::string readFile(const char* path) {
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void RenderFrame(Camera3D camera) {
    BeginDrawing();
    ClearBackground(backgroundColor);

    //BeginMode3D(camera);
    RenderScene(camera, g_instances);
    //EndMode3D();

    DrawText("WASD to move camera, Right Click to look around", 10, 10, 20, LIGHTGRAY);
    DrawText("Hold LSHIFT to move slower", 10, 40, 20, LIGHTGRAY);

    DrawRectangle(GetScreenWidth() - 30, GetScreenHeight() - 30, 20, 20, LIGHTGRAY);

    int fps = GetFPS();
    DrawText(TextFormat("FPS: %d", fps), 10, GetScreenHeight() - 30, 20, GREEN);

    EndDrawing();
}

int main(int argc, char** argv) {
    bool runHeadlessSingle = false;
    bool runHeadlessDiscover = false;
    const char* headlessScriptPath = nullptr;

    //Simple CLI:
    // BlockEngine --run <script.luau>   -> run script
    // BlockEngine --test                -> run lua/test_runner.luau (which discovers lua/tests/*.luau) then exit (no window)
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--run") == 0 && i + 1 < argc) {
            runHeadlessSingle = true;
            headlessScriptPath = argv[i + 1];
            ++i;
        } else if (strcmp(argv[i], "--test") == 0) {
            runHeadlessDiscover = true;
        }
    }

    DataModel* dataModel = DataModel::GetInstance();
    Workspace* workspace = dataModel->WorkspaceService;

    workspace->ChildAdded.Connect([](Instance* child) {
        if (auto* part = dynamic_cast<BasePart*>(child)) {
            if (std::find(g_instances.begin(), g_instances.end(), part) == g_instances.end()) {
                g_instances.push_back(part);
            }
        }
    });

    workspace->ChildRemoved.Connect([](Instance* child) {
        if (auto* part = dynamic_cast<BasePart*>(child)) {
            auto it = std::find(g_instances.begin(), g_instances.end(), part);
            if (it != g_instances.end()) {
                g_instances.erase(it);
            }
        }
    });

    L_main = luaL_newstate();
    luaL_openlibs(L_main);

    LuaBindings::RegisterScriptBindings(L_main, g_instances, g_camera);
    //Expose helper to Lua
    lua_pushcfunction(L_main, L_RunChunk, "__RUN_CHUNK");
    lua_setglobal(L_main, "__RUN_CHUNK");

    //Headless test mode (no window):
    if (runHeadlessDiscover) {
        printf("Running test runner: lua/test_runner.luau\n");
        std::string scriptText = readFile("lua/test_runner.luau");
        //Inject discovered test files into global __TEST_FILES for the runner
        lua_newtable(L_main);
        int idx = 1;
        std::error_code ec;
        for (auto& entry : std::filesystem::directory_iterator("./lua/tests", ec)) {
            if (ec) break;
            if (!entry.is_regular_file()) continue;
            auto p = entry.path();
            if (p.extension() == ".luau") {
                std::string pathStr = p.generic_string();
                std::string src = readFile(pathStr.c_str());

                lua_newtable(L_main);
                lua_pushstring(L_main, pathStr.c_str());
                lua_setfield(L_main, -2, "path");
                lua_pushlstring(L_main, src.c_str(), src.size());
                lua_setfield(L_main, -2, "source");
                lua_rawseti(L_main, -2, idx++);
            }
        }
        lua_setglobal(L_main, "__TEST_FILES");

        if (!Task_RunScript(L_main, scriptText)) {
            printf("Script execution failed.\n");
            return 1;
        }
        //Execute scheduled tasks to completion in headless mode
        bool ok = TaskScheduler_RunToIdle();
        return ok ? 0 : 1;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "BlockEngine");
    SetTargetFPS(500);

    PrepareRenderer();

    //Handle script run mode with window
    if (runHeadlessSingle && headlessScriptPath) {
        printf("Running script: %s\n", headlessScriptPath);
        std::string scriptText = readFile(headlessScriptPath);
        if (!Task_RunScript(L_main, scriptText)) {
            printf("Script execution failed.\n");
            CloseWindow();
            return 1;
        }
    }

    bool rotatingCamera = false;
    bool warpThisFrame = false;
    Vector2 anchorPos = {0, 0};
    Vector3 worldUp = {0, 1, 0};

    g_camera = {};
    g_camera.up = {0.0f, 1.0f, 0.0f};
    g_camera.fovy = 70.0f;
    g_camera.projection = CAMERA_PERSPECTIVE;
    g_camera.position = {0, 2, -5};
    g_camera.target = {0, 2, 0};

    Part* baseplate = new Part();
    baseplate->Color = Color3(Color{92, 92, 92, 0});
    baseplate->Position = Vector3Game{0, -8, 0};
    baseplate->Size = Vector3Game{2048, 16, 2048};
    baseplate->Name = "Baseplate";
    baseplate->SetParent(workspace);

    g_instances.push_back(baseplate);

    LoadSkybox();

    while (!WindowShouldClose()) {
        //const double time = GetTime();
        const double deltaTime = GetFrameTime();

        float mouseWheelDelta = GetMouseWheelMove();


        float moveSpeed = 25.0f * static_cast<float>(deltaTime);

        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            moveSpeed *= 0.25;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            anchorPos = GetMousePosition();
            rotatingCamera = true;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
            rotatingCamera = false;
        }

        if (rotatingCamera) {
            if (!warpThisFrame) {
                Vector2 delta = Vector2Subtract(GetMousePosition(), anchorPos);
                gYaw += delta.x * 0.004f;
                gPitch += -delta.y * 0.004f;

                const float limit = PI / 2 - 0.01f;
                if (gPitch > limit) gPitch = limit;
                if (gPitch < -limit) gPitch = -limit;
            } else {
                SetMousePosition((int)anchorPos.x, (int)anchorPos.y);
            }

            warpThisFrame = !warpThisFrame;
        }

        Vector3 forward = {
            cosf(gPitch) * cosf(gYaw),
            sinf(gPitch),
            cosf(gPitch) * sinf(gYaw)};

        Vector3 right = Vector3CrossProduct(worldUp, forward);
        if (Vector3LengthSqr(right) < 1e-6f) right = Vector3{1, 0, 0};
        right = Vector3Normalize(right);

        Vector3 up = Vector3CrossProduct(forward, right);

        Vector3 delta = {0, 0, 0};
        if (IsKeyDown(KEY_W)) delta = Vector3Add(delta, Vector3Scale(forward, moveSpeed));
        if (IsKeyDown(KEY_S)) delta = Vector3Subtract(delta, Vector3Scale(forward, moveSpeed));
        if (IsKeyDown(KEY_D)) delta = Vector3Subtract(delta, Vector3Scale(right, moveSpeed));
        if (IsKeyDown(KEY_A)) delta = Vector3Add(delta, Vector3Scale(right, moveSpeed));
        if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_E)) delta = Vector3Add(delta, Vector3Scale(up, moveSpeed));
        if (IsKeyDown(KEY_Q)) delta = Vector3Subtract(delta, Vector3Scale(up, moveSpeed));

        g_camera.position = Vector3Add(g_camera.position, delta);
        g_camera.target = Vector3Add(g_camera.position, forward);
        g_camera.up = up;

        TaskScheduler_Step();
        RenderFrame(g_camera);
    }

    //Cleanup
    delete dataModel;

    UnloadPrimitiveModels();

    g_tasks.clear();
    g_instances.clear();

    UnloadSkybox();
    CloseWindow();
    return 0;
}
