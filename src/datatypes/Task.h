#pragma once
#include <algorithm>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

#include "raylib.h"
#include "raymath.h"

#include <lua.h>
#include <luacode.h>
#include <lualib.h>

/**
 * @brief Task scheduler for managing asynchronous Lua coroutines
 * @description The task library provides functions for scheduling and managing
 * asynchronous execution of code. Tasks can be spawned to run concurrently and
 * can yield execution with wait() to pause for a specified duration.
 * @example
 * ```lua
 * -- Spawn a task that runs asynchronously
 * task.spawn(function()
 *     print("Starting task")
 *     task.wait(2)
 *     print("Task finished after 2 seconds")
 * end)
 *
 * -- Wait in the current context
 * print("Before wait")
 * task.wait(1)
 * print("After 1 second wait")
 * ```
 */
struct LuaTask {
    lua_State *thread;
    double WakeTime = 0.0;
    double SleepStartTime = 0.0;
    bool Finished = false;

    LuaTask(lua_State *L) : thread(lua_newthread(L)) {}
};

/**
 * @internal
 */
struct SleepingTask {
    lua_State *thread;
    double wakeTime; // in seconds
};

extern std::vector<LuaTask> g_tasks;

int Task_RunScript(lua_State *L, std::string &scriptText);

/**
 * @method spawn
 * @param func function - The function to run asynchronously
 * @returns void
 * @description Schedules a function to run in a new task without yielding the
 * current thread
 * @example
 * ```lua
 * task.spawn(function()
 *     for i = 1, 5 do
 *         print("Task iteration:", i)
 *         task.wait(1)
 *     end
 * end)
 * print("This prints immediately")
 * ```
 */

/**
 * @method wait
 * @param duration number - The time to wait in seconds (default: 0)
 * @returns number
 * @description Yields the current task for the specified duration and returns
 * the actual elapsed time
 * @example
 * ```lua
 * print("Starting")
 * local elapsed = task.wait(2.5)
 * print("Waited for", elapsed, "seconds")
 * ```
 */

void TaskScheduler_Step();
bool TaskScheduler_RunToIdle();
void Task_Bind(lua_State *L);