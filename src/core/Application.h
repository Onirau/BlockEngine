#pragma once

#include "bgfx/defines.h"
#include <cmath>
#include <filesystem>
#include <vector>

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <lua.h>
#include <luacode.h>
#include <lualib.h>

#include "../Global.h"
#include "PrimitiveModels.h"
#include "SkyboxRenderer.h"

class Application {
protected:
    GLFWwindow *window = nullptr;
    bool rotatingCamera = false;
    bool warpThisFrame = false;
    glm::vec2 anchorPos = {0, 0};
    glm::vec3 worldUp = {0, 1, 0};
    float gYaw = 0.0f;
    float gPitch = 0.0f;
    DataModel *dataModel = nullptr;
    Workspace *workspace = nullptr;

    int windowWidth = 1280;
    int windowHeight = 720;
    double lastTime = 0.0;
    double deltaTime = 0.0;

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

        if (window) {
            bgfx::shutdown();
            glfwDestroyWindow(window);
            glfwTerminate();
        }

        lua_close(L_main);
    }

    void Run() {
        Initialize();

        if (!glfwInit()) {
            return;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(windowWidth, windowHeight, GetWindowTitle(),
                                  nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            return;
        }

        // Setup bgfx
        bgfx::Init init;
        init.type = bgfx::RendererType::Direct3D11;

#if defined(_WIN32)
        init.platformData.nwh = glfwGetWin32Window(window);
#elif defined(__linux__)
        init.platformData.ndt = glfwGetX11Display();
        init.platformData.nwh = (void *)(uintptr_t)glfwGetX11Window(window);
#endif

        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        init.resolution.width = windowWidth;
        init.resolution.height = windowHeight;
        init.resolution.reset = BGFX_RESET_VSYNC;

        if (!bgfx::init(init)) {
            glfwDestroyWindow(window);
            glfwTerminate();
            return;
        }

        // TEMP
        bgfx::setDebug(BGFX_DEBUG_STATS);

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xffffffff,
                           1.0f, 0);
        bgfx::setViewRect(0, 0, 0, windowWidth, windowHeight);

        // Setup callbacks
        glfwSetWindowUserPointer(window, this);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
        glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

        L_main = luaL_newstate();
        luaL_openlibs(L_main);

        LuaBindings::RegisterScriptBindings(L_main, g_instances, g_camera);

        PostLuaInitialize();

        PrepareRenderer();
        // LoadSkybox();

        // Initialize camera
        g_camera = {};
        g_camera.position = glm::vec3{0, 2, -5};
        g_camera.target = glm::vec3{0, 2, 0};
        g_camera.up = glm::vec3{0.0f, 1.0f, 0.0f};
        g_camera.fov = 70.0f;

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

        lastTime = glfwGetTime();
        MainLoop();
        Cleanup();
    }

protected:
    virtual const char *GetWindowTitle() const = 0;

private:
    static void MouseButtonCallback(GLFWwindow *win, int button, int action,
                                    int mods) {
        Application *app =
            static_cast<Application *>(glfwGetWindowUserPointer(win));
        if (!app)
            return;

        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) {
                double x, y;
                glfwGetCursorPos(win, &x, &y);
                app->anchorPos = glm::vec2(x, y);
                app->rotatingCamera = true;
            } else if (action == GLFW_RELEASE) {
                app->rotatingCamera = false;
            }
        }
    }

    static void FramebufferSizeCallback(GLFWwindow *win, int width,
                                        int height) {
        Application *app =
            static_cast<Application *>(glfwGetWindowUserPointer(win));
        if (!app)
            return;

        app->windowWidth = width;
        app->windowHeight = height;
        bgfx::reset(width, height, BGFX_RESET_VSYNC);
        bgfx::setViewRect(0, 0, 0, width, height);
    }

    void MainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            double currentTime = glfwGetTime();
            deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            float moveSpeed = 25.0f * static_cast<float>(deltaTime);

            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                moveSpeed *= 0.25f;
            }

            if (rotatingCamera) {
                if (!warpThisFrame) {
                    double mx, my;
                    glfwGetCursorPos(window, &mx, &my);
                    glm::vec2 mousePos(mx, my);
                    glm::vec2 delta = mousePos - anchorPos;

                    gYaw += delta.x * 0.004f;
                    gPitch += -delta.y * 0.004f;

                    const float limit = glm::pi<float>() / 2.0f - 0.01f;
                    gPitch = glm::clamp(gPitch, -limit, limit);
                } else {
                    glfwSetCursorPos(window, anchorPos.x, anchorPos.y);
                }
                warpThisFrame = !warpThisFrame;
            }

            glm::vec3 forward = {cosf(gPitch) * cosf(gYaw), sinf(gPitch),
                                 cosf(gPitch) * sinf(gYaw)};

            glm::vec3 right = glm::cross(worldUp, forward);
            if (glm::dot(right, right) < 1e-6f) {
                right = glm::vec3(1, 0, 0);
            }
            right = glm::normalize(right);

            glm::vec3 up = glm::cross(forward, right);

            glm::vec3 delta(0, 0, 0);
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                delta += forward * moveSpeed;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                delta -= forward * moveSpeed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                delta -= right * moveSpeed;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                delta += right * moveSpeed;
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
                delta += up * moveSpeed;
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
                delta -= up * moveSpeed;

            g_camera.position += delta;
            g_camera.target = g_camera.position + forward;
            g_camera.up = up;

            TaskScheduler_Step();

            // Set view and projection matrices
            glm::mat4 view =
                glm::lookAt(g_camera.position, g_camera.target, g_camera.up);
            glm::mat4 proj = glm::perspective(
                glm::radians(g_camera.fov),
                (float)windowWidth / (float)windowHeight, 0.1f, 1000.0f);

            bgfx::setViewTransform(0, glm::value_ptr(view),
                                   glm::value_ptr(proj));
            bgfx::touch(0);

            // RenderScene(g_camera, g_instances);
            RenderUI();

            bgfx::frame();
        }
    }
};