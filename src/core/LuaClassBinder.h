#pragma once

#include "../../luau/VM/include/lua.h"
#include "../../luau/VM/include/lualib.h"
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

//Forward declarations
struct Instance;

//Property accessor types
using PropertyGetter = std::function<int(lua_State*, Instance*)>;
using PropertySetter = std::function<int(lua_State*, Instance*, int)>;//int is value index

//Method type
using MethodFunc = std::function<int(lua_State*, Instance*)>;

struct PropertyDescriptor {
    PropertyGetter getter;
    PropertySetter setter;
    bool readonly = false;
};

struct ClassDescriptor {
    std::string className;
    std::string parentClassName;
    std::unordered_map<std::string, PropertyDescriptor> properties;
    std::unordered_map<std::string, MethodFunc> methods;
    std::function<Instance*()> constructor = nullptr;
};

class LuaClassBinder {
private:
    static std::unordered_map<std::string, ClassDescriptor> s_classes;

    static int GenericIndex(lua_State* L);
    static int GenericNewIndex(lua_State* L);
    static int GenericToString(lua_State* L);
    static int GenericEq(lua_State* L);
    static int GenericGC(lua_State* L);
    static int GenericConstructor(lua_State* L);

    static std::string GetMetatableName(const std::string& className);
    static void CreateMetatable(lua_State* L, const std::string& className);

public:
    //Register a class with inheritance
    static void RegisterClass(const std::string& className,
                              const std::string& parentClassName = "");

    //Add property to a class
    static void AddProperty(const std::string& className,
                            const std::string& propName,
                            PropertyGetter getter,
                            PropertySetter setter = nullptr);

    //Add method to a class
    static void AddMethod(const std::string& className,
                          const std::string& methodName,
                          MethodFunc method);

    //Set constructor for a class
    static void SetConstructor(const std::string& className,
                               std::function<Instance*()> ctor);

    //Bind all registered classes to Lua
    static void BindAll(lua_State* L);

    //Check if userdata is of given class (including inheritance)
    static bool IsA(lua_State* L, int idx, const std::string& className);

    //Get instance from userdata (with type checking)
    static Instance* CheckInstance(lua_State* L, int idx, const std::string& className = "Instance");

    //Push instance to Lua stack
    static void PushInstance(lua_State* L, Instance* inst);

    //Get class descriptor (now public for external access)
    static ClassDescriptor* GetDescriptor(const std::string& className);
};

//Helper macros for cleaner property registration
#define LUA_PROPERTY(className, propName, type) \
    LuaClassBinder::AddProperty(#className, #propName, [](lua_State* L, Instance* inst) -> int { \
            auto* obj = static_cast<className*>(inst); \
            /* getter logic */ \
            return 1; }, [](lua_State* L, Instance* inst, int valueIdx) -> int { \
            auto* obj = static_cast<className*>(inst); \
            /* setter logic */ \
            return 0; })