#pragma once

enum class PartType : int {
    Ball = 0,
    Block,
    Cylinder,
    Wedge,
    CornerWedge,
};

inline constexpr const char *kPartTypeNames[] = {
    "Ball",
    "Block",
    "Cylinder",
    "Wedge",
    "CornerWedge",
};

inline constexpr int kPartTypeCount = sizeof(kPartTypeNames) / sizeof(kPartTypeNames[0]);


