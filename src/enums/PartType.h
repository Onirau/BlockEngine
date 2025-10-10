#pragma once

/**
 * @brief Enumeration of different part shapes available in the engine
 * @description PartType defines the geometric shape of a Part. Each type has
 * different collision properties and visual appearance.
 * @example
 * ```lua
 * local part = Instance.new("Part")
 * part.Shape = Enum.PartType.Ball
 *
 * -- Check part type
 * if part.Shape == Enum.PartType.Block then
 *     print("This is a block")
 * end
 * ```
 */
enum class PartType : int {
    Ball = 0,
    Block,
    Cylinder,
    Wedge,
    CornerWedge,
};

inline constexpr const char *kPartTypeNames[] = {
    "Ball", "Block", "Cylinder", "Wedge", "CornerWedge",
};

inline constexpr int kPartTypeCount =
    sizeof(kPartTypeNames) / sizeof(kPartTypeNames[0]);