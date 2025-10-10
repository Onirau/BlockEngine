#pragma once
#include "../core/Application.h"
#include "../core/Config.h"
#include <filesystem>
#include <fstream>
#include <sstream>

class EditorApplication : public Application {
private:
    bool headless = false;
    bool runTests = false;
    const char *scriptPath = nullptr;

    std::string readFile(const char *path) {
        std::ifstream file(path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    // Compile and run a Lua chunk from source in the current state.
    // Lua signature: __RUN_CHUNK(source: string, chunkname: string?) -> (bool
    // ok, string? err)
    static int L_RunChunk(lua_State *L) {
        size_t len = 0;
        const char *src = luaL_checklstring(L, 1, &len);
        const char *name = luaL_optstring(L, 2, "ScriptChunk");

        size_t bcSize = 0;
        lua_CompileOptions opts{};
        opts.optimizationLevel = 1;
        opts.debugLevel = 1;
        const char *bytecode = luau_compile(src, len, &opts, &bcSize);

        int loadStatus = luau_load(L, name, bytecode, bcSize, 0);
        if (loadStatus != LUA_OK) {
            const char *err = lua_tostring(L, -1);
            lua_pushboolean(L, 0);
            lua_pushstring(L, err ? err : "load error");
            return 2;
        }

        int status = lua_pcall(L, 0, 0, 0);
        if (status != LUA_OK) {
            const char *err = lua_tostring(L, -1);
            lua_pushboolean(L, 0);
            lua_pushstring(L, err ? err : "runtime error");
            return 2;
        }

        lua_pushboolean(L, 1);
        lua_pushnil(L);
        return 2;
    }

    bool RunTests() {
        printf("Running test runner: lua/test_runner.luau\n");
        std::string scriptText = readFile("lua/test_runner.luau");

        // Inject discovered test files into global __TEST_FILES for the runner
        lua_newtable(L_main);
        int idx = 1;
        std::error_code ec;
        for (auto &entry :
             std::filesystem::directory_iterator("./lua/tests", ec)) {
            if (ec)
                break;
            if (!entry.is_regular_file())
                continue;
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
            return false;
        }

        return TaskScheduler_RunToIdle();
    }

    bool RunScript() {
        printf("Running script: %s\n", scriptPath);
        std::string scriptText = readFile(scriptPath);
        if (!Task_RunScript(L_main, scriptText)) {
            printf("Script execution failed.\n");
            return false;
        }

        if (headless) {
            return TaskScheduler_RunToIdle();
        }
        return true;
    }

protected:
    void RenderUI() override {
        DrawText("WASD to move camera, Right Click to look around", 10, 10, 20,
                 LIGHTGRAY);
        DrawText("Hold LSHIFT to move slower", 10, 40, 20, LIGHTGRAY);
        DrawText(TextFormat("FPS: %d", GetFPS()), 10, GetScreenHeight() - 30,
                 20, GREEN);
        DrawRectangle(GetScreenWidth() - 30, GetScreenHeight() - 30, 20, 20,
                      LIGHTGRAY);
    }

    void Initialize() override {
        // Create baseplate
        Part *baseplate = new Part();
        baseplate->Color = Color3(Color{92, 92, 92, 0});
        baseplate->Position = Vector3Game{0, -8, 0};
        baseplate->Size = Vector3Game{2048, 16, 2048};
        baseplate->Name = "Baseplate";
        baseplate->SetParent(workspace);
        g_instances.push_back(baseplate);
    }

    void PostLuaInitialize() {
        // Expose helper to Lua (called after L_main is initialized)
        lua_pushcfunction(L_main, L_RunChunk, "__RUN_CHUNK");
        lua_setglobal(L_main, "__RUN_CHUNK");

        // Run script/tests if specified
        if (runTests) {
            bool result = RunTests();
            if (headless) {
                exit(result ? 0 : 1);
            }
        } else if (scriptPath) {
            bool result = RunScript();
            if (headless) {
                exit(result ? 0 : 1);
            }
        }
    }

    void Cleanup() override {}

    const char *GetWindowTitle() const override {
        return ENGINE_MAKE_WINDOW_TITLE("Editor");
    }

public:
    EditorApplication(int argc, char **argv) {
        // Parse command line arguments
        for (int i = 1; i < argc; ++i) {
            if (strcmp(argv[i], "--headless") == 0) {
                headless = true;
            } else if (strcmp(argv[i], "--test") == 0) {
                runTests = true;
            } else if (strcmp(argv[i], "--run") == 0 && i + 1 < argc) {
                scriptPath = argv[i + 1];
                ++i;
            }
        }
    }

    int Run() {
        if (headless && !runTests && !scriptPath) {
            printf(
                "Error: --headless requires either --test or --run <script>\n");
            return 1;
        }

        // Run
        Application::Run();
        return 0;
    }
};