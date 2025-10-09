#pragma once
#include "../core/Application.h"
#include "../core/Config.h"

class RuntimeApplication : public Application {
private:
    std::string placeFile;

protected:
    void RenderUI() override {
        DrawText("WASD to move camera, Right Click to look around", 10, 10, 20, LIGHTGRAY);
        DrawText("Hold LSHIFT to move slower", 10, 40, 20, LIGHTGRAY);
        DrawText(TextFormat("FPS: %d", GetFPS()), 10, GetScreenHeight() - 30, 20, GREEN);
    }

    void Initialize() override {
        //TODO: Implement place file loading
        //LoadPlaceFile(placeFile);

        //temp
        Part* baseplate = new Part();

        baseplate->Color = Color3(Color{92, 92, 92, 0});
        baseplate->Position = Vector3Game{0, -8, 0};
        baseplate->Size = Vector3Game{2048, 16, 2048};
        baseplate->Name = "Baseplate";
        baseplate->SetParent(workspace);

        g_instances.push_back(baseplate);
    }

    void Cleanup() override {
    }
    const char* GetWindowTitle() const override {
        return ENGINE_MAKE_WINDOW_TITLE("Runtime");
    }

public:
    RuntimeApplication(const std::string& placeFilePath)
        : placeFile(placeFilePath) {
    }
};