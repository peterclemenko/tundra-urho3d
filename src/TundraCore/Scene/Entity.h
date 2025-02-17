// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "SceneFwd.h"
#include "IComponent.h"
#include "IAttribute.h"
#include "EntityAction.h"
#include "UniqueIdGenerator.h"

#include <kNetFwd.h>

#include <Urho3D/Core/Object.h>

namespace Tundra
{

/// Represents a single object in a Scene.
/** An entity is a collection of components that define the data and the functionality of the entity.
    Entity can have multiple components of the same type as long as the component names are unique.

    Entities should not be directly created, instead use Scene::CreateEntity() et al.
    @ingroup Scene_group */
class TUNDRACORE_API Entity : public Object
{
    URHO3D_OBJECT(Entity, Object);

public:
    typedef HashMap<component_id_t, ComponentPtr> ComponentMap; ///< Component container.
    typedef Vector<ComponentPtr> ComponentVector; ///< Component vector container.
    typedef HashMap<String, EntityAction *> ActionMap; ///< Action container
    typedef Vector<EntityWeakPtr> ChildEntityVector; ///< Child entity vector container.

    /// If entity has components that are still alive, they become free-floating.
    ~Entity();

    /// Creates and returns a component with certain type and name (optional), already cast to the correct type.
    template <typename T>
    SharedPtr<T> CreateComponent(const String &name = "", AttributeChange::Type change = AttributeChange::Default, bool replicated = true);

    /// Creates, or returns, if already existing, a component with certain type and name (optional), already casted to the correct type.
    template<typename T>
    SharedPtr<T> GetOrCreateComponent(const String &name = "", AttributeChange::Type change = AttributeChange::Default, bool replicated = true);

    /// Returns a component with certain type, already cast to correct type, or empty pointer if component was not found
    /** If there are several components with the specified type, returns the first component found (arbitrary). */
    template <class T>
    SharedPtr<T> Component() const;

    /// Returns a component with certain type and name, already cast to correct type, or empty pointer if component was not found
    /** @param name name of the component */
    template <class T>
    SharedPtr<T> Component(const String& name) const;

    /** Returns list of components with certain class type, already cast to correct type.
        @param T Component class type.
        @return List of components with certain class type, or empty list if no components was found. */
    template <class T>
    Vector<SharedPtr<T> > ComponentsOfType() const;

    /// Sets group to this entity in Name component. If the component doesn't exist, it will be created.
    /** @param groupName Name of the group. */
    void SetGroup(const String &groupName);

    /// Returns group name this entity belongs to if Name is available, empty string otherwise.
    String Group() const;

    /// In the following, deserialization functions are now disabled since deserialization can't safely
    /// process the exact same data that was serialized, or it risks receiving entity ID conflicts in the scene.
    /// @todo Implement a deserialization flow that takes that into account. In the meanwhile, use Scene
    /// functions for achieving the same.

    void SerializeToBinary(kNet::DataSerializer &dst, bool serializeTemporary = false, bool serializeLocal = true, bool serializeChildren = true) const;
//        void DeserializeFromBinary(kNet::DataDeserializer &src, AttributeChange::Type change);

    /// Emit EnterView signal. Called by the rendering subsystem
    void EmitEnterView(IComponent* camera);
    
    /// Emit LeaveView signal. Called by the rendering subsystem
    void EmitLeaveView(IComponent* camera);

    /// Returns true if the two entities have the same id, false otherwise
    bool operator == (const Entity &other) const { return Id() == other.Id(); }

    /// Returns true if the two entities have different id, false otherwise
    bool operator != (const Entity &other) const { return !(*this == other); }

    /// comparison by id
    bool operator < (const Entity &other) const { return Id() < other.Id(); }

    /// Forcibly changes id of an existing component. If there already is a component with the new id, it will be purged
    /** @note Called by scenesync. This will not trigger any signals
        @param old_id Old id of the existing component
        @param new_id New id to set */
    void ChangeComponentId(component_id_t old_id, component_id_t new_id);
    
    /// Create a component with predefined ID. Called by SyncManager.
    /** @param typeId Unique type ID of the component.
        @param name name of the component */
    ComponentPtr CreateComponentWithId(component_id_t compId, u32 typeId, const String &name, AttributeChange::Type change = AttributeChange::Default);

    /// Introspection for the entity, returns all components
    const ComponentMap &Components() const { return components_; }

    /// Returns number of components.
    uint NumComponents() const { return components_.Size(); }

    /// Returns actions map for introspection/reflection.
    const ActionMap &Actions() const { return actions_; }

    /// @cond PRIVATE
    /// Do not directly allocate new entities using operator new, but use the factory-based Scene::CreateEntity functions instead.
    /** @param framework Framework.
        @param id unique ID for the entity.
        @param temporary Is the entity temporary.
        @param scene Scene this entity belongs to */
    Entity(Framework* framework, entity_id_t id, bool temporary, Scene* scene);
    /// @endcond

    /// Returns a component by ID. This is the fastest way to query, as the components are stored in a map by id.
    ComponentPtr ComponentById(component_id_t id) const;
    /// Returns a component with type 'typeName' or empty pointer if component was not found
    /** If there are several components with the specified type, returns the first component found (arbitrary).
        @param typeName type of the component. */
    ComponentPtr Component(const String &typeName) const;
    /// @overload
    /** @param typeId Component type ID. */
    ComponentPtr Component(u32 typeId) const;
    /// @overload
    /** @param name Specifies the name of the component to fetch. This can be used to distinguish between multiple instances of components of same type. */
    ComponentPtr Component(const String &typeName, const String &name) const;
    /// @overload
    /** @param typeId The type id of the component to get.
        @param name name of the component */
    ComponentPtr Component(u32 typeId, const String &name) const;

    /// Returns a component with type 'typeName' or creates & adds it if not found. If could not create, returns empty pointer
    /** @param typeName The type name of the component to create.
        @param change Change signaling mode, in case component has to be created
        @param replicated Whether new component will be replicated through network
        @return Pointer to the component, or an empty pointer if the component could be retrieved or created. */
    ComponentPtr GetOrCreateComponent(const String &typeName, AttributeChange::Type change = AttributeChange::Default, bool replicated = true);
    /// @overload
    /** @param name If specified, the component having the given name is returned, or created if it doesn't exist. */
    ComponentPtr GetOrCreateComponent(const String &typeName, const String &name, AttributeChange::Type change = AttributeChange::Default, bool replicated = true);
    /// @overload
    /** @param typeId Identifies the component type to create by the id of the type instead of the name. */
    ComponentPtr GetOrCreateComponent(u32 typeId, AttributeChange::Type change = AttributeChange::Default, bool replicated = true);
    /// @overload
    /** @param name name of the component */
    ComponentPtr GetOrCreateComponent(u32 typeId, const String &name, AttributeChange::Type change = AttributeChange::Default, bool replicated = true);

    /// Returns a component with type 'typeName' or creates & adds it as local if not found. If could not create, returns empty pointer
    ComponentPtr GetOrCreateLocalComponent(const String &typeName);
    /// @overload
    /** Returns a component with type 'typeName' and name 'name' or creates & adds it as local if not found. If could not create, returns empty pointer */
    ComponentPtr GetOrCreateLocalComponent(const String &typeName, const String &name);

    /// Creates a new component and attaches it to this entity. 
    /** @param typeName type of the component.
        @param change Change signaling mode, in case component has to be created
        @param replicated Whether new component will be replicated through network
        @return Returns a pointer to the newly created component, or null if creation failed. Common causes for failing to create an component
        is that a component with the same (typename, name) pair exists, or that components of the given typename are not recognized by the system. */
    ComponentPtr CreateComponent(const String &typeName, AttributeChange::Type change = AttributeChange::Default, bool replicated = true);
    /// @overload
    /** @param name name of the component */
    ComponentPtr CreateComponent(const String &typeName, const String &name, AttributeChange::Type change = AttributeChange::Default, bool replicated = true);
    /// @overload
    /** @param typeId Unique type ID of the component. */
    ComponentPtr CreateComponent(u32 typeId, AttributeChange::Type change = AttributeChange::Default, bool replicated = true);
    /// @overload
    /** @param typeId Unique type ID of the component.
        @param name name of the component */
    ComponentPtr CreateComponent(u32 typeId, const String &name, AttributeChange::Type change = AttributeChange::Default, bool replicated = true);
    
    /// Creates a local component with type 'typeName' and adds it to the entity. If could not create, return empty pointer
    ComponentPtr CreateLocalComponent(const String &typeName);
    /// @overload @param name The arbitrary name identifier for the component.
    ComponentPtr CreateLocalComponent(const String &typeName, const String &name);
    
    /// Attaches an existing parentless component to this entity.
    /** A component ID will be allocated.
        Entities can contain any number of components of any type.
        It is also possible to have several components of the same type,
        although in most cases it is probably not sensible.

        @param component The component to add to this entity. The component must be parentless, i.e.
                      previously created using SceneAPI::CreateComponent.
        @param change Change signaling mode */
    void AddComponent(const ComponentPtr &component, AttributeChange::Type change = AttributeChange::Default);
    /// @overload
    /** Attaches an existing parentless component to this entity, using the specific ID. This variant is used by SyncManager. */
    void AddComponent(component_id_t id, const ComponentPtr &component, AttributeChange::Type change = AttributeChange::Default);

    /// Removes component from this entity.
    /** @param component Pointer to the component to remove
        @param change Specifies how other parts of the system are notified of this removal.
        @sa RemoveComponentById */
    void RemoveComponent(const ComponentPtr &component, AttributeChange::Type change = AttributeChange::Default); /**< @overload */
    void RemoveComponent(const String &typeName, AttributeChange::Type change = AttributeChange::Default) { RemoveComponent(Component(typeName), change); } /**< @overload @param typeName The component type name. */
    void RemoveComponent(const String &typeName, const String &name, AttributeChange::Type change = AttributeChange::Default) { RemoveComponent(Component(typeName, name), change); }  /**< @overload */
    
    /// Removes component by ID.
    /** @sa RemoveComponent */
    void RemoveComponentById(component_id_t id, AttributeChange::Type change = AttributeChange::Default);
    
    /// Removes all components that match @c typeName and returns how many was removed.
    uint  RemoveComponents(const String &typeName, AttributeChange::Type change = AttributeChange::Default);
    
    /// Removes all components that match @c typeId and returns how many was removed.
    uint RemoveComponents(u32 typeId, AttributeChange::Type change = AttributeChange::Default);

    /// Removes all components from the entity.
    void RemoveAllComponents(AttributeChange::Type change = AttributeChange::Default);

    /// Returns list of components with specific type or an empty list if no components were found.
    /** @param typeId Component type ID. */
    ComponentVector ComponentsOfType(u32 typeId) const;
    /// @overload
    /** @param typeName Type name of the component.
        @note The overload taking component type ID is more efficient than this overload. */
    ComponentVector ComponentsOfType(const String &typeName) const;

    /// Creates clone of the entity.
    /** @param createAsLocal If true, the new entity will be local entity. If false, the entity will be replicated.
        @param createAsTemporary Will the new entity be temporary.
        @param name cloneName for the new entity.
        @param changeType Change signaling mode.
        @return Pointer to the new entity, or null pointer if the cloning fails. */
    EntityPtr Clone(bool createAsLocal, bool createAsTemporary, const String &cloneName= "", AttributeChange::Type changeType = AttributeChange::Default) const;

    /// Serializes this entity and its' components to the given XML document
    /** @param doc The XML document to serialize this entity to.
        @param base_element Points to the <scene> element of this XML document. This entity will be serialized as a child to base_element.
        @param serializeTemporary Serialize temporary entities or components for application-specific purposes. The default value is false. 
        @param bool serializeLocal Serialize local entities. Default true.
        @param serializeChildren Serialize child entities. Default true.*/
    void SerializeToXML(Urho3D::XMLFile& doc, Urho3D::XMLElement& base_element, bool serializeTemporary = false, bool serializeLocal = true, bool serializeChildren = true) const;
//        void DeserializeFromXML(Urho3D::XMLElement& element, AttributeChange::Type change);

    /// Serializes this entity, and returns the generated XML as a string
    /** @param serializeTemporary Serialize temporary entities for application-specific purposes. The default value is false.
        @param bool serializeLocal Serialize local entities. Default true.
        @param serializeChildren Serialize child entities. Default true.
        @param createSceneElement Whether to wrap the entity XML element in a scene XML element. Default false.
        @sa SerializeToXML */
    String SerializeToXMLString(bool serializeTemporary = false, bool serializeLocal = true, bool serializeChildren = true, bool createSceneElement = false) const;
//        bool DeserializeFromXMLString(const String &src, AttributeChange::Type change);

    /// Sets name of the entity to Name component. If the component doesn't exist, it will be created.
    /** @param name Name. */
    void SetName(const String &name);

    /// Returns name of this entity if Name is available, empty string otherwise.
    String Name() const;

    /// Sets description of the entity to Name component. If the component doesn't exist, it will be created.
    /** @param desc Description. */
    void SetDescription(const String &desc);

    /// Returns description of this entity if Name is available, empty string otherwise.
    String Description() const;

    /// Creates and registers new action for this entity, or returns an existing action.
    /** Use this function from scripting languages.
        @param name Name of the action, case-insensitive.
        @note Never returns null pointer
        @note Never store the returned pointer. */
    EntityAction *Action(const String &name);

    /// Removes an existing action.
    /** Use this function from scripting languages.
        @param name Name of the action, case-insensitive. */
    void RemoveAction(const String &name);

    /// Executes an arbitrary action for all components of this entity.
    /** The components may or may not handle the action.
        @param type Execution type(s), i.e. where the actions is executed.
        @param action Name of the action.
        @param p1 1st parameter for the action, if applicable.
        @param p2 2nd parameter for the action, if applicable.
        @param p3 3rd parameter for the action, if applicable. */
    void Exec(EntityAction::ExecTypeField type, const String &action, const String &p1 = "", const String &p2 = "", const String &p3 = "");
    /// @overload
    /** @param params List of parameters for the action. */
    void Exec(EntityAction::ExecTypeField type, const String &action, const StringVector &params);
    /// @overload
    /** Experimental overload using Variant. Converts the variants to strings.
        @note If called from JavaScript, syntax '<targetEntity>["Exec(EntityAction::ExecTypeField,String,VariantList)"](2, "name", params);' must be used. */
    void Exec(EntityAction::ExecTypeField type, const String &action, const VariantList &params);

    /// Sets whether entity is temporary. Temporary entities won't be saved when the scene is saved.
    /** By definition, all components of a temporary entity are temporary as well.
        @param change Change signaling mode. */
    void SetTemporary(bool enable, AttributeChange::Type change = AttributeChange::Default);

    /// Returns whether entity is temporary. Temporary entities won't be saved when the scene is saved.
    /** By definition, all components of a temporary entity are temporary as well. */
    bool IsTemporary() const { return temporary_; }

    /// Returns if this entity's changes will NOT be sent over the network.
    /// An Entity is always either local or replicated, but not both.
    bool IsLocal() const { return id_ >= UniqueIdGenerator::FIRST_LOCAL_ID; }

    /// Returns if this entity's changes will be sent over the network.
    /// An Entity is always either local or replicated, but not both.
    bool IsReplicated() const { return id_ < UniqueIdGenerator::FIRST_LOCAL_ID; }

    /// Returns if this entity is pending a proper ID assignment from the server.
    bool IsUnacked() const { return id_ >= UniqueIdGenerator::FIRST_UNACKED_ID && id_ < UniqueIdGenerator::FIRST_LOCAL_ID; }

    /// Returns the identifier string for the entity.
    /** Syntax of the string: 'Entity ID <id>' or 'Entity "<name>" (ID: <id>)' if entity has a name. */
    String ToString() const;

    /// Returns the unique id of this entity
    entity_id_t Id() const { return id_; }

    /// Returns framework
    Framework *GetFramework() const { return framework_; }

    /// Returns parent scene of this entity.
    Scene* ParentScene() const { return scene_; }

    /// Add an entity as a child.
    /** When an entity is added as a child, its rendering transform in the Placeable component will automatically follow the parent entity,
        unless the Placeable component specifies an alternate parent (such as a bone) to follow. Child entities are also 
        automatically removed from the scene when the parent entity is removed.
        If the child already is parented to another entity, the existing parent assignment is removed.
        @param entity Entity to add as a child.
        @param change Change signaling mode. Normally this should be Default or Replicate, so that parent assignment is propagated over the network. */
    void AddChild(EntityPtr child, AttributeChange::Type change = AttributeChange::Default);

    /// Remove a child entity.
    /** Will remove the child entity from the scene. If the intention is to re-parent the entity, AddChild() to the new parent
        should be called instead. 
        @param child Child entity to remove.
        @param change Change signaling mode. */
    void RemoveChild(EntityPtr child, AttributeChange::Type change = AttributeChange::Default);

    /// Remove all child entities.
    void RemoveAllChildren(AttributeChange::Type change = AttributeChange::Default);

    /// Detach a child entity to the scene root level (make it parentless) without removing it from the scene.
    /** @param child Child entity to detach.
        @param change Change signaling mode. */
    void DetachChild(EntityPtr child, AttributeChange::Type change = AttributeChange::Default);

    /// Set the parent entity of the entity. Null parent will set to the scene root level (default.)
    /** Same as calling AddChild() on the new parent entity, or DetachChild() in case of setting a null parent.
        @param parent New parent entity.
        @param change Change signaling mode. */
    void SetParent(EntityPtr parent, AttributeChange::Type change = AttributeChange::Default);

    /// Creates new child entity that contains the specified components.
    /** To create an empty entity, omit the components parameter.
        @param id Id of the new entity. Specify 0 to use the next free (replicated) ID, see also NextFreeId and NextFreeIdLocal.
        @param components Optional list of component names the entity will use. If omitted or the list is empty, creates an empty entity.
        @param change Notification/network replication mode
        @param replicated Whether entity is replicated. Default true.
        @param componentsReplicated Whether components will be replicated, true by default.
        @param temporary Will the entity be temporary i.e. it is no serialized to disk by default, false by default. */
    EntityPtr CreateChild(entity_id_t id = 0, const StringVector &components = StringVector(),
        AttributeChange::Type change = AttributeChange::Default, bool replicated = true, bool componentsReplicated = true, bool temporary = false);

    /// Creates new local child entity that contains the specified components
    /** To create an empty entity omit components parameter.
        @param components Optional list of component names the entity will use. If omitted or the list is empty, creates an empty entity.
        @param change Notification/network replication mode
        @param componentsReplicated Whether components will be replicated, false by default, but components of local entities are not replicated so this has no effect.
        @param temporary Will the entity be temporary i.e. it is no serialized to disk by default. */
    EntityPtr CreateLocalChild(const StringVector &components = StringVector(),
        AttributeChange::Type change = AttributeChange::Default, bool componentsReplicated = false, bool temporary = false);

    /// Returns parent entity of this entity, or null if entity is on the root level.
    EntityPtr Parent() const { return parent_.Lock(); }

    /// Returns if parent entity is set.
    bool HasParent() const { return parent_.Get() != nullptr; }

    /// Returns number of child entities.
    uint NumChildren() const { return children_.Size(); }

    /// Returns child entity by index.
    EntityPtr Child(uint index) const;

    /// Returns child entity by name. Optionally recursive.
    EntityPtr ChildByName(const String& name, bool recursive = false) const;

    /// Returns child entities. Optionally recursive
    EntityVector Children(bool recursive = false) const;

    /// Helper function for determinating whether or not this entity should be serialized with the provided serialization options.
    bool ShouldBeSerialized(bool serializeTemporary, bool serializeLocal, bool serializeChildren) const;

    /// A component has been added to the entity
    /** @note When this signal is received on new entity creation, the attributes might not be filled yet! */ 
    Signal2<IComponent*, AttributeChange::Type> ComponentAdded;

    /// A component has been removed from the entity
    /** @note When this signal is received on new entity creation, the attributes might not be filled yet! */ 
    Signal2<IComponent*, AttributeChange::Type> ComponentRemoved;

    /// Signal when this entity is deleted
    Signal2<Entity*, AttributeChange::Type> EntityRemoved;

    /// Signal when this entity's temporary state has been toggled
    Signal2<Entity*, AttributeChange::Type> TemporaryStateToggled;

    /// The entity has entered a camera's view. Triggered by the rendering subsystem.
    Signal1<IComponent*> EnterView;

    /// The entity has left a camera's view. Triggered by the rendering subsystem.
    Signal1<IComponent*> LeaveView;

    /// The entity's parent has changed.
    Signal3<Entity*, Entity*, AttributeChange::Type> ParentChanged;

private:
    friend class Scene;

    /// Set new id
    void SetNewId(entity_id_t id) { id_ = id; }

    /// Set new scene
    void SetScene(Scene* scene) { scene_ = scene; }

    /// Emit a entity deletion signal. Called from Scene
    void EmitEntityRemoved(AttributeChange::Type change);

    /// Remove a component by iterator. Called internally
    void RemoveComponent(ComponentMap::Iterator iter, AttributeChange::Type change);

    /// Collect child entities into an entity list, optionally recursive.
    void CollectChildren(EntityVector& children, bool recursive) const;

    UniqueIdGenerator idGenerator_; ///< Component ID generator
    ComponentMap components_; ///< a list of all components
    entity_id_t id_; ///< Unique id for this entity
    Framework* framework_; ///< Pointer to framework
    Scene* scene_; ///< Pointer to scene
    ActionMap actions_; ///< Map of registered entity actions.
    bool temporary_; ///< Temporary-flag

    ChildEntityVector children_; ///< Child entities. Note that the entities are authoritatively owned by the scene; the child reference is weak intentionally.
    EntityWeakPtr parent_; ///< Parent entity. Note that the entities are authoritatively owned by the scene; the parent reference is weak intentionally.
};

/// Represents weak pointer to Transform attribute.
struct TransformAttributeWeakPtr : public AttributeWeakPtr
{
    /** @param p If the placeable component is parented, pointer to the parent placeable entity. */
    TransformAttributeWeakPtr(IComponent* c, IAttribute *a, const EntityPtr &p) :
        AttributeWeakPtr(c, a),
        parentPlaceableEntity(p)
    {
    }
    /// If the placeable component is parented, points to the parent placeable entity.
    EntityWeakPtr parentPlaceableEntity;
};

}

#include "Entity.inl"
