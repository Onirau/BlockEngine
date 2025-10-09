#pragma once
#include "../core/Application.h"
#include "../core/Config.h"

class EditorApplication : public Application {
protected:
    void RenderUI() override {
        DrawText("WASD to move camera, Right Click to look around", 10, 10, 20, LIGHTGRAY);
        DrawText("Hold LSHIFT to move slower", 10, 40, 20, LIGHTGRAY);
        DrawText(TextFormat("FPS: %d", GetFPS()), 10, GetScreenHeight() - 30, 20, GREEN);
        DrawRectangle(GetScreenWidth() - 30, GetScreenHeight() - 30, 20, 20, LIGHTGRAY);
    }

    void Initialize() override {
    }
    void Cleanup() override {
    }
    const char* GetWindowTitle() const override {
        return ENGINE_MAKE_WINDOW_TITLE("Editor");
    }
};