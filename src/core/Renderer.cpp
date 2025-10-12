#include "Renderer.h"

#include "../enums/PartType.h"
#include "../instances/DataModel.h"
#include "../instances/Part.h"
#include "../instances/services/Lighting.h"
#include "PrimitiveModels.h"
#include "SkyboxRenderer.h"
#include <direct.h>

#define SHADOW_MAP_SIZE 2048

unsigned int depthFBO, depthTex;
Shader shadowShader;
int lightSpaceMatrixLoc;
int shadowMapLoc;
int lightDirLoc;

Texture2D g_defaultTexture;

static Color Color3ToColor(const Color3 &c) {
    return Color{(unsigned char)roundf(c.r * 255.0f),
                 (unsigned char)roundf(c.g * 255.0f),
                 (unsigned char)roundf(c.b * 255.0f), 255};
}

Texture2D GenerateDefaultTexture(int width, int height) {
    Image img = GenImageColor(width, height, BLANK);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char n = (unsigned char)(rand() % 16 + 239);
            Color c = {n, n, n, 255};
            ImageDrawPixel(&img, x, y, c);
        }
    }

    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);

    return tex;
}

void DrawPart(const Part part) {
    rlPushMatrix();

    rlTranslatef(part.Position.x, part.Position.y, part.Position.z);
    rlRotatef(part.Rotation.x, 1, 0, 0);
    rlRotatef(part.Rotation.y, 0, 1, 0);
    rlRotatef(part.Rotation.z, 0, 0, 1);
    rlScalef(part.Size.x, part.Size.y, part.Size.z);

    Color color = Color3ToColor(part.Color);
    Model *model = nullptr;

    if (part.Shape == "Block") {
        model = GetPrimitiveModel(PartType::Block);
    } else if (part.Shape == "Sphere") {
        model = GetPrimitiveModel(PartType::Ball);
    } else if (part.Shape == "Cylinder") {
        model = GetPrimitiveModel(PartType::Cylinder);
    } else if (part.Shape == "Wedge") {
        model = GetPrimitiveModel(PartType::Wedge);
    } else if (part.Shape == "CornerWedge") {
        model = GetPrimitiveModel(PartType::CornerWedge);
    } else {
        model = GetPrimitiveModel(PartType::Block);
    }

    if (model) {
        model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
            g_defaultTexture;
        DrawModel(*model, {0, 0, 0}, 1.0f, color);
    }

    rlPopMatrix();
}

// Simple depth-only rendering for shadow map
void DrawPartDepthOnly(const Part part) {
    rlPushMatrix();

    rlTranslatef(part.Position.x, part.Position.y, part.Position.z);
    rlRotatef(part.Rotation.x, 1, 0, 0);
    rlRotatef(part.Rotation.y, 0, 1, 0);
    rlRotatef(part.Rotation.z, 0, 0, 1);
    rlScalef(part.Size.x, part.Size.y, part.Size.z);

    Model *model = nullptr;

    if (part.Shape == "Block") {
        model = GetPrimitiveModel(PartType::Block);
    } else if (part.Shape == "Sphere") {
        model = GetPrimitiveModel(PartType::Ball);
    } else if (part.Shape == "Cylinder") {
        model = GetPrimitiveModel(PartType::Cylinder);
    } else if (part.Shape == "Wedge") {
        model = GetPrimitiveModel(PartType::Wedge);
    } else if (part.Shape == "CornerWedge") {
        model = GetPrimitiveModel(PartType::CornerWedge);
    } else {
        model = GetPrimitiveModel(PartType::Block);
    }

    if (model) {
        // Draw just the mesh without materials/textures
        DrawMesh(model->meshes[0], model->materials[0], MatrixIdentity());
    }

    rlPopMatrix();
}

void DrawPartWithShadows(const Part part, Shader shader) {
    rlPushMatrix();

    rlTranslatef(part.Position.x, part.Position.y, part.Position.z);
    rlRotatef(part.Rotation.x, 1, 0, 0);
    rlRotatef(part.Rotation.y, 0, 1, 0);
    rlRotatef(part.Rotation.z, 0, 0, 1);
    rlScalef(part.Size.x, part.Size.y, part.Size.z);

    Color color = Color3ToColor(part.Color);
    Model *model = nullptr;

    if (part.Shape == "Block") {
        model = GetPrimitiveModel(PartType::Block);
    } else if (part.Shape == "Sphere") {
        model = GetPrimitiveModel(PartType::Ball);
    } else if (part.Shape == "Cylinder") {
        model = GetPrimitiveModel(PartType::Cylinder);
    } else if (part.Shape == "Wedge") {
        model = GetPrimitiveModel(PartType::Wedge);
    } else if (part.Shape == "CornerWedge") {
        model = GetPrimitiveModel(PartType::CornerWedge);
    } else {
        model = GetPrimitiveModel(PartType::Block);
    }

    if (model) {
        model->materials[0].shader = shader;
        model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
            g_defaultTexture;
        DrawModel(*model, {0, 0, 0}, 1.0f, color);
    }

    rlPopMatrix();
}

void DrawSun(const Vector3 &lightDir, const Camera3D &camera) {
    // Calculate sun position (much farther away)
    Vector3 sunPos = Vector3Scale(Vector3Negate(lightDir), 5000.0f);
    sunPos = Vector3Add(camera.position, sunPos);

    rlPushMatrix();
    rlTranslatef(sunPos.x, sunPos.y, sunPos.z);

    DrawSphere(Vector3{0, 0, 0}, 50.0f, Color{255, 240, 200, 255});

    rlPopMatrix();
}

void RenderScene(Camera3D camera, const std::vector<BasePart *> instances) {
    DataModel *dataModel = DataModel::GetInstance();
    Lighting *lighting =
        static_cast<Lighting *>(dataModel->GetService("Lighting"));

    Vector3Game sunDirGame = lighting->GetSunDirection();

    Vector3 lightDir = {sunDirGame.x, sunDirGame.y, sunDirGame.z};
    lightDir = Vector3Normalize(Vector3Negate(lightDir));

    Color3 ambientColor = lighting->Ambient;
    Vector3 sceneCenter = {0.0f, 0.0f, 0.0f};

    RenderShadowMap(instances, lightDir, sceneCenter);

    rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());

    BeginMode3D(camera);
    DrawSkybox();
    DrawSun(lightDir, camera);

    // Change Far Plane
    Matrix projection = MatrixPerspective(
        camera.fovy * DEG2RAD,
        (float)GetScreenWidth() / (float)GetScreenHeight(), 0.1f, 10000.0f);
    rlSetMatrixProjection(projection);

    // Set shadow shader uniforms
    Matrix lightSpaceMatrix = GetLightSpaceMatrix(lightDir, sceneCenter);
    SetShaderValueMatrix(shadowShader, lightSpaceMatrixLoc, lightSpaceMatrix);

    // Set light direction
    float lightDirArray[3] = {lightDir.x, lightDir.y, lightDir.z};
    SetShaderValue(shadowShader, lightDirLoc, lightDirArray,
                   SHADER_UNIFORM_VEC3);

    // Bind shadow map to texture unit 1
    rlActiveTextureSlot(1);
    rlEnableTexture(depthTex);
    int shadowMapUnit = 1;
    SetShaderValue(shadowShader, shadowMapLoc, &shadowMapUnit,
                   SHADER_UNIFORM_INT);
    rlActiveTextureSlot(0);

    // Draw all parts with shadow shader
    for (BasePart *inst : instances) {
        if (inst->ClassName == "Part") {
            Part *p = dynamic_cast<Part *>(inst);
            if (p) {
                DrawPartWithShadows(*p, shadowShader);
            }
        }
    }

    EndMode3D();
}

void PrepareShadowMap() {
    depthTex = rlLoadTextureDepth(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, true);

    // Set texture parameters for shadow map
    rlTextureParameters(depthTex, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_CLAMP);
    rlTextureParameters(depthTex, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_CLAMP);
    rlTextureParameters(depthTex, RL_TEXTURE_MIN_FILTER,
                        RL_TEXTURE_FILTER_LINEAR);
    rlTextureParameters(depthTex, RL_TEXTURE_MAG_FILTER,
                        RL_TEXTURE_FILTER_LINEAR);

    // Create framebuffer
    depthFBO = rlLoadFramebuffer();
    rlEnableFramebuffer(depthFBO);

    // Attach depth texture
    rlFramebufferAttach(depthFBO, depthTex, RL_ATTACHMENT_DEPTH,
                        RL_ATTACHMENT_TEXTURE2D, 0);

    // Check if framebuffer is complete
    if (rlFramebufferComplete(depthFBO)) {
        TraceLog(LOG_INFO, "Shadow map framebuffer created successfully");
    } else {
        TraceLog(LOG_ERROR, "Shadow map framebuffer creation failed");
    }

    rlDisableFramebuffer();
}

Matrix GetLightSpaceMatrix(const Vector3 &lightDir,
                           const Vector3 &sceneCenter) {
    // Position light camera away from scene center
    Vector3 lightPos = {sceneCenter.x - lightDir.x * 100.0f,
                        sceneCenter.y - lightDir.y * 100.0f,
                        sceneCenter.z - lightDir.z * 100.0f};

    // Create view matrix (look at scene from light)
    Matrix view =
        MatrixLookAt(lightPos, sceneCenter, Vector3{0.0f, 1.0f, 0.0f});

    // Create orthographic projection for directional light
    float orthoSize = 50.0f;
    Matrix projection =
        MatrixOrtho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 200.0f);

    return MatrixMultiply(view, projection);
}

void RenderShadowMap(const std::vector<BasePart *> &instances,
                     const Vector3 &lightDir, const Vector3 &sceneCenter) {
    // Enable shadow map framebuffer
    rlEnableFramebuffer(depthFBO);
    rlViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    rlClearScreenBuffers();

    // Use BeginMode3D to properly set up the matrices
    Camera3D lightCamera = {};
    Vector3 lightPos = {sceneCenter.x - lightDir.x * 100.0f,
                        sceneCenter.y - lightDir.y * 100.0f,
                        sceneCenter.z - lightDir.z * 100.0f};

    lightCamera.position = lightPos;
    lightCamera.target = sceneCenter;
    lightCamera.up = Vector3{0.0f, 1.0f, 0.0f};
    lightCamera.fovy = 100.0f; // This will be overridden by ortho projection
    lightCamera.projection = CAMERA_ORTHOGRAPHIC;

    BeginMode3D(lightCamera);

    // Override with orthographic projection
    Matrix projection = MatrixOrtho(-50.0f, 50.0f, -50.0f, 50.0f, 0.1f, 200.0f);
    rlSetMatrixProjection(projection);

    // Draw all parts (depth only)
    for (BasePart *inst : instances) {
        if (inst->ClassName == "Part") {
            Part *p = dynamic_cast<Part *>(inst);
            if (p) {
                DrawPartDepthOnly(*p);
            }
        }
    }

    EndMode3D();

    // Restore default framebuffer
    rlDisableFramebuffer();
}

void PrepareRenderer() {
    g_defaultTexture = GenerateDefaultTexture();
    PrepareShadowMap();
    PreparePrimitiveModels();

    // Load shadow shader
    shadowShader = LoadShader("src/shaders/shadow.vs", "src/shaders/shadow.fs");

    lightSpaceMatrixLoc = GetShaderLocation(shadowShader, "lightSpaceMatrix");
    shadowMapLoc = GetShaderLocation(shadowShader, "shadowMap");
    lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
}

void UnprepareRenderer() {
    rlUnloadFramebuffer(depthFBO);
    rlUnloadTexture(depthTex);
    UnloadTexture(g_defaultTexture);
    UnloadShader(shadowShader);
}