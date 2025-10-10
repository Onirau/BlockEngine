#pragma once

#include <cmath>
#include <filesystem>
#include <vector>

#include <lua.h>
#include <luacode.h>
#include <lualib.h>

#include "raylib.h"
#include "raymath.h"

#include "../Global.h"

class Application {
protected:
    bool rotatingCamera = false;
    bool warpThisFrame = false;
    Vector2 anchorPos = {0, 0};
    Vector3 worldUp = {0, 1, 0};
    float gYaw = 0.0f;
    float gPitch = 0.0f;
    DataModel *dataModel = nullptr;
    Workspace *workspace = nullptr;

    virtual void RenderUI() = 0;
    virtual void Initialize() = 0;
    virtual void PostLuaInitialize() {}
    virtual void Cleanup() = 0;

public:
    Application() {
        dataModel = DataModel::GetInstance();
        workspace = dataModel->WorkspaceService;
    }

    virtual ~Application() {
        delete dataModel;
        UnloadPrimitiveModels();
        g_instances.clear();
        UnloadSkybox();
        CloseWindow();
        lua_close(L_main);
    }

    void Run() {
        Initialize();

        workspace->ChildAdded.Connect([](Instance *child) {
            if (auto *part = dynamic_cast<BasePart *>(child)) {
                if (std::find(g_instances.begin(), g_instances.end(), part) ==
                    g_instances.end()) {
                    g_instances.push_back(part);
                }
            }
        });

        workspace->ChildRemoved.Connect([](Instance *child) {
            if (auto *part = dynamic_cast<BasePart *>(child)) {
                auto it =
                    std::find(g_instances.begin(), g_instances.end(), part);
                if (it != g_instances.end()) {
                    g_instances.erase(it);
                }
            }
        });

        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(1280, 720, GetWindowTitle());
        SetTargetFPS(60);

        L_main = luaL_newstate();
        luaL_openlibs(L_main);

        LuaBindings::RegisterScriptBindings(L_main, g_instances, g_camera);

        // Call post-Lua initialization hook
        PostLuaInitialize();

        PrepareRenderer();
        LoadSkybox();

        // Initialize camera
        g_camera = {};
        g_camera.position = Vector3{0, 2, -5};
        g_camera.target = Vector3{0, 2, 0};
        g_camera.up = Vector3{0.0f, 1.0f, 0.0f};
        g_camera.fovy = 70.0f;
        g_camera.projection = CAMERA_PERSPECTIVE;

        MainLoop();
        Cleanup();
    }

protected:
    virtual const char *GetWindowTitle() const = 0;

private:
    void MainLoop() {
        while (!WindowShouldClose()) {
            const double deltaTime = GetFrameTime();
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
                    Vector2 delta =
                        Vector2Subtract(GetMousePosition(), anchorPos);
                    gYaw += delta.x * 0.004f;
                    gPitch += -delta.y * 0.004f;

                    const float limit = PI / 2 - 0.01f;
                    if (gPitch > limit)
                        gPitch = limit;
                    if (gPitch < -limit)
                        gPitch = -limit;
                } else {
                    SetMousePosition((int)anchorPos.x, (int)anchorPos.y);
                }
                warpThisFrame = !warpThisFrame;
            }

            Vector3 forward = {cosf(gPitch) * cosf(gYaw), sinf(gPitch),
                               cosf(gPitch) * sinf(gYaw)};

            Vector3 right = Vector3CrossProduct(worldUp, forward);
            if (Vector3LengthSqr(right) < 1e-6f)
                right = Vector3{1, 0, 0};
            right = Vector3Normalize(right);

            Vector3 up = Vector3CrossProduct(forward, right);

            Vector3 delta = {0, 0, 0};
            if (IsKeyDown(KEY_W))
                delta = Vector3Add(delta, Vector3Scale(forward, moveSpeed));
            if (IsKeyDown(KEY_S))
                delta =
                    Vector3Subtract(delta, Vector3Scale(forward, moveSpeed));
            if (IsKeyDown(KEY_D))
                delta = Vector3Subtract(delta, Vector3Scale(right, moveSpeed));
            if (IsKeyDown(KEY_A))
                delta = Vector3Add(delta, Vector3Scale(right, moveSpeed));
            if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_E))
                delta = Vector3Add(delta, Vector3Scale(up, moveSpeed));
            if (IsKeyDown(KEY_Q))
                delta = Vector3Subtract(delta, Vector3Scale(up, moveSpeed));

            g_camera.position = Vector3Add(g_camera.position, delta);
            g_camera.target = Vector3Add(g_camera.position, forward);
            g_camera.up = up;

            TaskScheduler_Step();

            BeginDrawing();
            ClearBackground(RAYWHITE);

            RenderScene(g_camera, g_instances);
            RenderUI();

            EndDrawing();
        }
    }
};