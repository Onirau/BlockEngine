#pragma once
#include "raylib.h"
#include "raymath.h"
#include <cstring>
#include <unordered_map>

enum class PrimitiveShape {
    Block,
    Sphere,
    Cylinder,
    Wedge,
    CornerWedge,
};

void InitPrimitiveModels();
void UnloadPrimitiveModels();
Model *GetPrimitiveModel(PrimitiveShape shape);
