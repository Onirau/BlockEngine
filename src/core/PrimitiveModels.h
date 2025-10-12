#pragma once
#include "../enums/PartType.h"
#include "Model.h"
#include <cstring>
#include <unordered_map>

void PreparePrimitiveModels();
void UnloadPrimitiveModels();
Model *GetPrimitiveModel(PartType shape);