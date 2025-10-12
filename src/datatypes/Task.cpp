#include "Task.h"

std::vector<LuaTask> g_tasks;

int Task_RunScript(lua_State *L, std::string &scriptText) {
    size_t bcSize;
    lua_CompileOptions opts{};
    opts.optimizationLevel = 1;
    opts.debugLevel = 1;

    const char *bytecode =
        luau_compile(scriptText.c_str(), scriptText.size(), &opts, &bcSize);

    LuaTask task(L);
    lua_State *thread = task.thread;

    // Prepare per-script environment and pass it to luau_load
    lua_newtable(thread);                    // env
    lua_newtable(thread);                    // mt
    lua_pushvalue(thread, LUA_GLOBALSINDEX); // _G
    lua_setfield(thread, -2, "__index");
    lua_setmetatable(thread, -2); // setmetatable(env, mt)

    int envIndex = lua_gettop(thread);
    int loadStatus =
        luau_load(thread, "ScriptChunk", bytecode, bcSize, envIndex);
    lua_remove(thread, envIndex); // remove env, keep function
    if (loadStatus != LUA_OK) {
        const char *err = lua_tostring(thread, -1);
        printf("Error loading script: %s\n", err);
        lua_pop(thread, 1);
        return 0;
    }

    task.WakeTime = glfwGetTime();
    g_tasks.push_back(std::move(task));

    return 1;
}

static int Task_Spawn(lua_State *L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);

    LuaTask t(L);
    lua_pushvalue(L, 1);
    lua_xmove(L, t.thread, 1);

    g_tasks.push_back(t);

    return 0;
}

static int Task_Wait(lua_State *L) {
    double delay = luaL_optnumber(L, 1, 0.0);
    double now = glfwGetTime();

    for (auto &task : g_tasks) {
        if (task.thread == L) {
            task.SleepStartTime = now;
            task.WakeTime = now + delay;
            return lua_yield(L, 0);
        }
    }

    luaL_error(L, "attempted to use task.wait outside of a running task");
    return 0;
}

void TaskScheduler_Step() {
    double now = glfwGetTime();

    for (auto &task : g_tasks) {
        if (task.Finished)
            continue;

        if (now >= task.WakeTime) {
            double elapsed = now - task.SleepStartTime;
            lua_pushnumber(task.thread, elapsed); // return value of task.wait()

            int status = lua_resume(task.thread, nullptr, 1);

            if (status == LUA_YIELD) {
                // will be resumed later
            } else if (status == LUA_OK) {
                task.Finished = true;
            } else {
                printf("Lua error: %s\n", lua_tostring(task.thread, -1));
                lua_pop(task.thread, 1);
                task.Finished = true;
            }
        }
    }

    g_tasks.erase(std::remove_if(g_tasks.begin(), g_tasks.end(),
                                 [](const LuaTask &t) { return t.Finished; }),
                  g_tasks.end());
}

// Run until no runnable tasks remain
bool TaskScheduler_RunToIdle() {
    // Keep stepping until all tasks finished or no progress for a short time

    size_t lastRemaining = SIZE_MAX;
    int stagnation = 0;
    for (;;) {
        size_t remaining = 0;
        for (const auto &t : g_tasks)
            if (!t.Finished)
                ++remaining;
        if (remaining == 0)
            return true;
        if (remaining == lastRemaining) {
            if (++stagnation > 200)
                return false; //~200 iterations without progress
        } else {
            stagnation = 0;
            lastRemaining = remaining;
        }
        TaskScheduler_Step();
    }
}

void Task_Bind(lua_State *L) {
    lua_newtable(L);

    lua_pushcfunction(L, Task_Spawn, "spawn");
    lua_setfield(L, -2, "spawn");
    lua_pushcfunction(L, Task_Wait, "wait");
    lua_setfield(L, -2, "wait");

    lua_setglobal(L, "task");
}
