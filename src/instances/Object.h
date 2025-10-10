#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <memory>
#include <algorithm>
#include "../core/Signal.h"

//Forward declaration for Lua
struct lua_State;

struct Object {
    //-- Properties --//
    std::string ClassName;
    std::string Name;

    //-- Events --//
    Signal Changed;

    //-- Methods --//
    Object(const std::string& className = "Object");
    virtual ~Object();
    virtual bool IsA(const std::string& className) const;
    virtual void FirePropertyChanged(const std::string& propertyName);
};

//Lua binding function declaration
void Object_Bind(lua_State* L);