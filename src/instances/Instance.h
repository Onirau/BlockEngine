#pragma once

#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include "../core/Signal.h"
#include "../datatypes/Color3.h"
#include "../datatypes/Vector3.h"
#include "Object.h"

struct Attribute {
    std::string Name;
    std::variant<bool, double, std::string, Vector3Game, Color3> Value;
};

/**
 * @class Instance
 * @brief Base class for all objects in the game hierarchy
 *
 * @description
 * Instance is the fundamental building block of the game's object tree.
 * It provides parent-child relationships, attributes, and events for tracking
 * changes in the hierarchy. All game objects inherit from Instance.
 *
 * @inherits Object
 *
 */
struct Instance : public Object {
    //-- Properties --//

    /**
     * @property Parent
     * @type Instance | nil
     * @default nil
     * @description The parent of this instance in the hierarchy
     */
    Instance *Parent = nullptr;

    std::vector<Instance *> Children;
    std::vector<Attribute> Attributes;

    /**
     * @property Archivable
     * @type bool
     * @default true
     * @description When true, the instance can be saved and cloned
     */
    bool Archivable = true;

    /**
     * @property Name
     * @type string
     * @default ClassName
     * @description The name of this instance
     */
    std::string Name = ClassName;

    //-- Events --//

    /**
     * @event AncestryChanged
     * @param child Instance
     * @param parent Instance
     * @description Fires when the instance's ancestry changes
     */
    Signal AncestryChanged;

    /**
     * @event AttributeChanged
     * @param attributeName string
     * @description Fires when an attribute is changed
     */
    Signal AttributeChanged;

    /**
     * @event ChildAdded
     * @param child Instance
     * @description Fires when a child is added to this instance
     */
    Signal ChildAdded;

    /**
     * @event ChildRemoved
     * @param child Instance
     * @description Fires when a child is removed from this instance
     */
    Signal ChildRemoved;

    /**
     * @event DescendantAdded
     * @param descendant Instance
     * @description Fires when a descendant is added anywhere in the tree
     */
    Signal DescendantAdded;

    /**
     * @event DescendantRemoving
     * @param descendant Instance
     * @description Fires when a descendant is about to be removed
     */
    Signal DescendantRemoving;

    /**
     * @event Destroying
     * @description Fires when this instance is being destroyed
     */
    Signal Destroying;

    //-- Methods --//
    Instance(const std::string &className = "Instance");
    virtual ~Instance();

    /**
     * @method AddTag
     * @param tag string
     * @description Adds a tag to this instance
     */
    void AddTag(std::string &tag);

    /**
     * @method HasTag
     * @param tag string
     * @returns bool
     * @description Checks if this instance has a specific tag
     */
    bool HasTag(std::string &tag);

    /**
     * @method RemoveTag
     * @param tag string
     * @description Removes a tag from this instance
     */
    void RemoveTag(std::string &tag);

    /**
     * @method GetAttribute
     * @param attribute string
     * @returns Attribute | nil
     * @description Gets the value of an attribute
     */
    std::optional<Attribute> GetAttribute(std::string &attribute);

    /**
     * @method GetAttributes
     * @returns table
     * @description Returns a table of all attributes
     */
    std::vector<Attribute> GetAttributes();

    /**
     * @method SetAttribute
     * @param attribute string
     * @param value Attribute
     * @description Sets the value of an attribute
     *
     * @example
     * ```lua
     * part:SetAttribute("Health", 100)
     * local health = part:GetAttribute("Health")
     * ```
     */
    void SetAttribute(std::string &attribute, Attribute value);

    /**
     * @method FindFirstAncestor
     * @param name string
     * @returns Instance | nil
     * @description Finds the first ancestor with the specified name
     */
    std::optional<Instance *> FindFirstAncestor(std::string &name);

    /**
     * @method FindFirstAncestorOfClass
     * @param className string
     * @returns Instance | nil
     * @description Finds the first ancestor of the specified class
     */
    std::optional<Instance *> FindFirstAncestorOfClass(std::string &className);

    /**
     * @method FindFirstAncestorWhichIsA
     * @param className string
     * @returns Instance | nil
     * @description Finds the first ancestor that is an instance of the class
     */
    std::optional<Instance *> FindFirstAncestorWhichIsA(std::string &className);

    /**
     * @method FindFirstChild
     * @param name string
     * @returns Instance | nil
     * @description Finds the first direct child with the specified name
     */
    std::optional<Instance *> FindFirstChild(std::string &name);

    /**
     * @method FindFirstChildOfClass
     * @param name string
     * @returns Instance | nil
     * @description Finds the first direct child of the specified class
     */
    std::optional<Instance *> FindFirstChildOfClass(std::string &name);

    /**
     * @method FindFirstChildWhichIsA
     * @param name string
     * @returns Instance | nil
     * @description Finds the first child that is an instance of the class
     */
    std::optional<Instance *> FindFirstChildWhichIsA(std::string &name);

    /**
     * @method FindFirstDescendant
     * @param name string
     * @returns Instance | nil
     * @description Finds the first descendant with the specified name
     */
    std::optional<Instance *> FindFirstDescendant(std::string &name);

    /**
     * @method GetChildren
     * @returns table
     * @description Returns an array of all direct children
     *
     * @example
     * ```lua
     * for _, child in ipairs(workspace:GetChildren()) do
     *     print(child.Name)
     * end
     * ```
     */
    std::vector<Instance *> GetChildren();

    /**
     * @method GetDescendants
     * @returns table
     * @description Returns an array of all descendants
     */
    std::vector<Instance *> GetDescendants();

    /**
     * @method IsAncestorOf
     * @param descendant Instance
     * @returns bool
     * @description Checks if this instance is an ancestor of the target
     */
    bool IsAncestorOf(Instance *descendant);

    /**
     * @method IsDescendantOf
     * @param ancestor Instance
     * @returns bool
     * @description Checks if this instance is a descendant of the target
     */
    bool IsDescendantOf(Instance *ancestor);

    /**
     * @method ClearAllChildren
     * @description Removes and destroys all children
     */
    void ClearAllChildren();

    /**
     * @method Destroy
     * @description Permanently destroys this instance and all descendants
     */
    void Destroy();

    /**
     * @method SetParent
     * @param newParent Instance | nil
     * @description Sets the parent of this instance
     */
    void SetParent(Instance *newParent);

    /**
     * @method AddChild
     * @param child Instance
     * @description Adds a child to this instance
     */
    void AddChild(Instance *child);

    /**
     * @method RemoveChild
     * @param child Instance
     * @description Removes a child from this instance
     */
    void RemoveChild(Instance *child);

    virtual bool IsA(const std::string &className) const;
};

void Class_Instance_Bind(lua_State *L);