#pragma once

#include "../Global.h"
#include "../instances/BasePart.h"
#include "Texture2D.h"
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

extern Texture2D g_defaultTexture;

Texture2D GenerateDefaultTexture(int width = 256, int height = 256);

glm::mat4 GetLightSpaceMatrix(const glm::vec3 &lightDir,
                              const glm::vec3 &sceneCenter);

void RenderShadowMap(const std::vector<BasePart *> &instances,
                     const glm::vec3 &lightDir, const glm::vec3 &sceneCenter);
void PrepareRenderer();
void RenderScene(const Camera &camera,
                 const std::vector<BasePart *> &instances);
void UnprepareRenderer();