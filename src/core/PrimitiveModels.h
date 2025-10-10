#pragma once
#include "../enums/PartType.h"
#include "raylib.h"
#include "raymath.h"
#include <cstring>
#include <unordered_map>

void InitPrimitiveModels();
void UnloadPrimitiveModels();
Model *GetPrimitiveModel(PartType shape);
