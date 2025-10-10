#pragma once
#include <iostream>
#include <string>

#include "BasePart.h"

extern const char *validShapes[];

struct Part : public BasePart {
    std::string Shape = "Wedge";

    Part();

    Part(const std::string &name, const Vector3Game &position,
         const Vector3Game &size, const Color3 &color, bool anchored,
         std::string shape = "Wedge");

    virtual bool IsA(const std::string &className) const;
};

void Part_Bind(lua_State *L);
