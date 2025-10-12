#pragma once
#include <vector>

#include "raylib.h"
#include "rlgl.h"

#include "../instances/BasePart.h"

extern Texture2D g_defaultTexture;

Texture2D GenerateDefaultTexture(int width = 128, int height = 128);

Matrix GetLightSpaceMatrix(const Vector3 &lightDir, const Vector3 &sceneCenter);
void RenderShadowMap(const std::vector<BasePart *> &instances,
                     const Vector3 &lightDir, const Vector3 &sceneCenter);
void PrepareRenderer();
void RenderScene(Camera3D g_camera, std::vector<BasePart *> g_instances);
void UnprepareRenderer();