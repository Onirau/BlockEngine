#include "Renderer.h"

#include "../enums/PartType.h"
#include "../instances/DataModel.h"
#include "../instances/Part.h"
#include "../instances/services/Lighting.h"
#include "PrimitiveModels.h"
#include "SkyboxRenderer.h"

#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define SHADOW_MAP_SIZE 2048

// BGFX handles
bgfx::FrameBufferHandle shadowMapFB = BGFX_INVALID_HANDLE;
bgfx::TextureHandle depthTexture = BGFX_INVALID_HANDLE;
bgfx::ProgramHandle shadowProgram = BGFX_INVALID_HANDLE;
bgfx::ProgramHandle mainProgram = BGFX_INVALID_HANDLE;
bgfx::UniformHandle u_lightSpaceMatrix = BGFX_INVALID_HANDLE;
bgfx::UniformHandle u_shadowMap = BGFX_INVALID_HANDLE;
bgfx::UniformHandle u_lightDir = BGFX_INVALID_HANDLE;
bgfx::UniformHandle u_modelViewProj = BGFX_INVALID_HANDLE;
bgfx::UniformHandle u_model = BGFX_INVALID_HANDLE;
bgfx::UniformHandle u_color = BGFX_INVALID_HANDLE;

Texture2D g_defaultTexture;

struct Color {
    unsigned char r, g, b, a;
};

static Color Color3ToColor(const Color3 &c) {
    return Color{(unsigned char)roundf(c.r * 255.0f),
                 (unsigned char)roundf(c.g * 255.0f),
                 (unsigned char)roundf(c.b * 255.0f), 255};
}

// Helper to load shader from file
static bgfx::ShaderHandle loadShader(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file)
        return BGFX_INVALID_HANDLE;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    const bgfx::Memory *mem = bgfx::alloc(size + 1);
    fread(mem->data, 1, size, file);
    mem->data[size] = '\0';
    fclose(file);

    return bgfx::createShader(mem);
}

Texture2D GenerateDefaultTexture(int width, int height) {
    std::vector<unsigned char> data(width * height * 4);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char n = (unsigned char)(rand() % 16 + 239);
            int idx = (y * width + x) * 4;
            data[idx + 0] = n;
            data[idx + 1] = n;
            data[idx + 2] = n;
            data[idx + 3] = 255;
        }
    }

    const bgfx::Memory *mem = bgfx::makeRef(data.data(), width * height * 4);
    bgfx::TextureHandle handle = bgfx::createTexture2D(
        width, height, false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_NONE,
        mem);

    return Texture2D{handle, width, height};
}

void DrawPart(const Part &part, const glm::mat4 &viewProj,
              bgfx::ProgramHandle program) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(
        model, glm::vec3(part.Position.x, part.Position.y, part.Position.z));
    model =
        glm::rotate(model, glm::radians(part.Rotation.x), glm::vec3(1, 0, 0));
    model =
        glm::rotate(model, glm::radians(part.Rotation.y), glm::vec3(0, 1, 0));
    model =
        glm::rotate(model, glm::radians(part.Rotation.z), glm::vec3(0, 0, 1));
    model = glm::scale(model, glm::vec3(part.Size.x, part.Size.y, part.Size.z));

    glm::mat4 mvp = viewProj * model;

    Color color = Color3ToColor(part.Color);
    float colorVec[4] = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f,
                         color.a / 255.0f};

    PartType type = PartType::Block;
    if (part.Shape == "Sphere")
        type = PartType::Ball;
    else if (part.Shape == "Cylinder")
        type = PartType::Cylinder;
    else if (part.Shape == "Wedge")
        type = PartType::Wedge;
    else if (part.Shape == "CornerWedge")
        type = PartType::CornerWedge;

    Model *model_ptr = GetPrimitiveModel(type);
    if (!model_ptr)
        return;

    bgfx::setVertexBuffer(0, model_ptr->vbh);
    bgfx::setIndexBuffer(model_ptr->ibh);

    bgfx::setUniform(u_modelViewProj, glm::value_ptr(mvp));
    bgfx::setUniform(u_model, glm::value_ptr(model));
    bgfx::setUniform(u_color, colorVec);

    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
                   BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS |
                   BGFX_STATE_CULL_CW);

    bgfx::submit(0, program);
}

// void DrawPartDepthOnly(const Part &part, const glm::mat4 &lightSpaceMatrix) {
//     glm::mat4 model = glm::mat4(1.0f);
//     model = glm::translate(
//         model, glm::vec3(part.Position.x, part.Position.y, part.Position.z));
//     model =
//         glm::rotate(model, glm::radians(part.Rotation.x), glm::vec3(1, 0,
//         0));
//     model =
//         glm::rotate(model, glm::radians(part.Rotation.y), glm::vec3(0, 1,
//         0));
//     model =
//         glm::rotate(model, glm::radians(part.Rotation.z), glm::vec3(0, 0,
//         1));
//     model = glm::scale(model, glm::vec3(part.Size.x, part.Size.y,
//     part.Size.z));

//     glm::mat4 mvp = lightSpaceMatrix * model;

//     PartType type = PartType::Block;
//     if (part.Shape == "Sphere")
//         type = PartType::Ball;
//     else if (part.Shape == "Cylinder")
//         type = PartType::Cylinder;
//     else if (part.Shape == "Wedge")
//         type = PartType::Wedge;
//     else if (part.Shape == "CornerWedge")
//         type = PartType::CornerWedge;

//     PrimitiveModel *model_ptr = GetPrimitiveModel(type);
//     if (!model_ptr)
//         return;

//     bgfx::setVertexBuffer(0, model_ptr->vbh);
//     bgfx::setIndexBuffer(model_ptr->ibh);

//     bgfx::setUniform(u_modelViewProj, glm::value_ptr(mvp));

//     bgfx::setState(BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS);

//     bgfx::submit(0, shadowProgram);
// }

// void DrawSun(const glm::vec3 &lightDir, const glm::vec3 &cameraPos,
//              const glm::mat4 &viewProj) {
//     glm::vec3 sunPos = cameraPos - lightDir * 5000.0f;

//     glm::mat4 model = glm::translate(glm::mat4(1.0f), sunPos);
//     model = glm::scale(model, glm::vec3(50.0f));

//     glm::mat4 mvp = viewProj * model;

//     float color[4] = {1.0f, 0.94f, 0.78f, 1.0f};

//     PrimitiveModel *sphere = GetPrimitiveModel(PartType::Ball);
//     if (!sphere)
//         return;

//     bgfx::setVertexBuffer(0, sphere->vbh);
//     bgfx::setIndexBuffer(sphere->ibh);

//     bgfx::setUniform(u_modelViewProj, glm::value_ptr(mvp));
//     bgfx::setUniform(u_color, color);

//     bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
//                    BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS);

//     bgfx::submit(0, mainProgram);
// }

glm::mat4 GetLightSpaceMatrix(const glm::vec3 &lightDir,
                              const glm::vec3 &sceneCenter) {
    glm::vec3 lightPos = sceneCenter - lightDir * 100.0f;

    glm::mat4 view =
        glm::lookAt(lightPos, sceneCenter, glm::vec3(0.0f, 1.0f, 0.0f));

    float orthoSize = 50.0f;
    glm::mat4 projection =
        glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 200.0f);

    return projection * view;
}

// void RenderShadowMap(const std::vector<BasePart *> &instances,
//                      const glm::vec3 &lightDir, const glm::vec3 &sceneCenter)
//                      {
//     glm::mat4 lightSpaceMatrix = GetLightSpaceMatrix(lightDir, sceneCenter);

//     bgfx::setViewFrameBuffer(0, shadowMapFB);
//     bgfx::setViewRect(0, 0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
//     bgfx::setViewClear(0, BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);

//     for (BasePart *inst : instances) {
//         if (inst->ClassName == "Part") {
//             Part *p = dynamic_cast<Part *>(inst);
//             if (p) {
//                 DrawPartDepthOnly(*p, lightSpaceMatrix);
//             }
//         }
//     }
// }

void RenderScene(const Camera &camera,
                 const std::vector<BasePart *> &instances) {
    // Get view and projection matrices from camera
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 proj = camera.getProjectionMatrix();
    glm::mat4 viewProj = proj * view;

    DataModel *dataModel = DataModel::GetInstance();
    Lighting *lighting =
        static_cast<Lighting *>(dataModel->GetService("Lighting"));

    Vector3Game sunDirGame = lighting->GetSunDirection();
    glm::vec3 lightDir =
        glm::normalize(-glm::vec3(sunDirGame.x, sunDirGame.y, sunDirGame.z));

    Color3 ambientColor = lighting->Ambient;
    glm::vec3 sceneCenter(0.0f, 0.0f, 0.0f);

    // Render shadow map
    // RenderShadowMap(instances, lightDir, sceneCenter);

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f,
                       0);
    bgfx::setViewFrameBuffer(0, BGFX_INVALID_HANDLE);

    // Set view transform (already done in Application.h, but setting again for
    // clarity)
    bgfx::setViewTransform(0, glm::value_ptr(view), glm::value_ptr(proj));

    // Draw skybox
    // DrawSkybox(viewProj);

    // Draw sun
    // DrawSun(lightDir, camera.position, viewProj);

    // Set shadow uniforms
    // glm::mat4 lightSpaceMatrix = GetLightSpaceMatrix(lightDir, sceneCenter);
    // bgfx::setUniform(u_lightSpaceMatrix, glm::value_ptr(lightSpaceMatrix));

    // float lightDirArray[3] = {lightDir.x, lightDir.y, lightDir.z};
    // bgfx::setUniform(u_lightDir, lightDirArray);

    // bgfx::setTexture(1, u_shadowMap, depthTexture);

    // Draw all parts
    for (BasePart *inst : instances) {
        if (inst->ClassName == "Part") {
            Part *p = dynamic_cast<Part *>(inst);
            if (p) {
                DrawPart(*p, viewProj, mainProgram);
            }
        }
    }
}

// void PrepareShadowMap() {
//     depthTexture = bgfx::createTexture2D(
//         SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, false, 1,
//         bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT |
//         BGFX_TEXTURE_COMPARE_LEQUAL);

//     bgfx::TextureHandle fbtextures[] = {depthTexture};
//     shadowMapFB = bgfx::createFrameBuffer(1, fbtextures, true);

//     if (!bgfx::isValid(shadowMapFB)) {
//         printf("ERROR: Shadow map framebuffer creation failed\n");
//     } else {
//         printf("INFO: Shadow map framebuffer created successfully\n");
//     }
// }

void PrepareRenderer() {
    g_defaultTexture = GenerateDefaultTexture();
    // PrepareShadowMap();
    PreparePrimitiveModels();

    // Load shaders
    // bgfx::ShaderHandle vsh = loadShader("shaders/shadow_vs.bin");
    // bgfx::ShaderHandle fsh = loadShader("shaders/shadow_fs.bin");
    // shadowProgram = bgfx::createProgram(vsh, fsh, true);

    bgfx::ShaderHandle mainVsh = loadShader("shaders/main_vs.bin");
    bgfx::ShaderHandle mainFsh = loadShader("shaders/main_fs.bin");

    if (!bgfx::isValid(mainVsh) || !bgfx::isValid(mainFsh)) {
        printf("ERROR: Failed to load main shaders\n");
        return;
    }

    mainProgram = bgfx::createProgram(mainVsh, mainFsh, true);

    if (!bgfx::isValid(mainProgram)) {
        printf("ERROR: Failed to create main program\n");
        return;
    }

    // Create uniforms
    u_lightSpaceMatrix =
        bgfx::createUniform("u_lightSpaceMatrix", bgfx::UniformType::Mat4);
    u_shadowMap =
        bgfx::createUniform("u_shadowMap", bgfx::UniformType::Sampler);
    u_lightDir = bgfx::createUniform("u_lightDir", bgfx::UniformType::Vec4);
    u_modelViewProj =
        bgfx::createUniform("u_modelViewProj", bgfx::UniformType::Mat4);
    u_model = bgfx::createUniform("u_model", bgfx::UniformType::Mat4);
    u_color = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
}

void UnprepareRenderer() {
    if (bgfx::isValid(shadowMapFB))
        bgfx::destroy(shadowMapFB);

    if (bgfx::isValid(depthTexture))
        bgfx::destroy(depthTexture);

    if (bgfx::isValid(g_defaultTexture.handle))
        bgfx::destroy(g_defaultTexture.handle);

    if (bgfx::isValid(shadowProgram))
        bgfx::destroy(shadowProgram);

    if (bgfx::isValid(mainProgram))
        bgfx::destroy(mainProgram);

    if (bgfx::isValid(u_lightSpaceMatrix))
        bgfx::destroy(u_lightSpaceMatrix);

    if (bgfx::isValid(u_shadowMap))
        bgfx::destroy(u_shadowMap);

    if (bgfx::isValid(u_lightDir))
        bgfx::destroy(u_lightDir);

    if (bgfx::isValid(u_modelViewProj))
        bgfx::destroy(u_modelViewProj);

    if (bgfx::isValid(u_model))
        bgfx::destroy(u_model);

    if (bgfx::isValid(u_color))
        bgfx::destroy(u_color);
}