#pragma once
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

struct Skybox {
    bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle texture = BGFX_INVALID_HANDLE; // Cubemap texture
    bgfx::UniformHandle s_texCube = BGFX_INVALID_HANDLE;
};

extern Skybox g_skybox;

void LoadSkybox();
void UnloadSkybox();
void DrawSkybox();