#pragma once
#include <tuple>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <type_traits>
#include "include/component.h"
#include "include/utils/container_utils.h"

namespace entities {

/**
 * @brief Base template for all archetype types
 * 
 * Archetypes define a collection of components that can be attached to entities.
 * Each archetype type should inherit from this template.
 * 
 * @tparam Components The component types that make up this archetype
 */
template<typename... Components>
class Archetype {
public:
    // Create components for an entity
    static void Create(const std::string& entity_id) {
        (CreateComponent<Components>(entity_id), ...);
    }

    // Destroy components for an entity
    static void DestroyFor(const std::string& entity_id) {
        (DestroyComponent<Components>(entity_id), ...);
    }

    // Check if an entity has all components of this archetype
    static bool HasComponents(const std::string& entity_id) {
        return (HasComponent<Components>(entity_id) && ...);
    }

    // Get a specific component for an entity
    template<typename T>
    static T* GetComponent(const std::string& entity_id) {
        static_assert((std::is_same_v<T, Components> || ...), 
            "Component type not in archetype");
        auto it = component_maps<T>.find(entity_id);
        if (it != component_maps<T>.end()) {
            return it->second;
        }
        return nullptr;
    }

    // Get all entities that have a specific component
    template<typename T>
    static std::vector<std::string> GetEntities() {
        static_assert((std::is_same_v<T, Components> || ...), 
            "Component type not in archetype");
        return utils::SetToVector(entity_sets<T>);
    }

    // Get all components of a specific type
    template<typename T>
    static std::vector<T*> GetComponents() {
        static_assert((std::is_same_v<T, Components> || ...), 
            "Component type not in archetype");
        return utils::MapValuesToVector(component_maps<T>);
    }

    // Get the raw pointer to the component pool
    template<typename T>
    static T* GetComponentsPtr() {
        static_assert((std::is_same_v<T, Components> || ...), 
            "Component type not in archetype");
        return T::GetComponentsPtr();
    }

private:
    // Maps to store components by entity ID
    template<typename T>
    static inline std::map<std::string, T*> component_maps;

    // Sets to store entity IDs by component type
    template<typename T>
    static inline std::set<std::string> entity_sets;

    // Helper to create a component for an entity
    template<typename T>
    static void CreateComponent(const std::string& entity_id) {
        T* component = T::Create();
        component_maps<T>[entity_id] = component;
        entity_sets<T>.insert(entity_id);
        T::RegisterOwner(entity_id, component);
    }

    // Helper to destroy a component for an entity
    template<typename T>
    static void DestroyComponent(const std::string& entity_id) {
        auto it = component_maps<T>.find(entity_id);
        if (it != component_maps<T>.end()) {
            T::Destroy(it->second);
            component_maps<T>.erase(it);
            entity_sets<T>.erase(entity_id);
            T::UnregisterOwner(entity_id);
        }
    }

    // Helper to check if an entity has a specific component
    template<typename T>
    static bool HasComponent(const std::string& entity_id) {
        return component_maps<T>.find(entity_id) != component_maps<T>.end();
    }
};

// Macro to define an archetype
#define DEFINE_ARCHETYPE(Name, ...) \
    using Name = entities::Archetype<__VA_ARGS__>

/*
 * Example usage:
 * 
 * DEFINE_ARCHETYPE(PlayerArchetype,
 *     PositionComponent,
 *     VelocityComponent,
 *     PlayerComponent
 * );
 * 
 * // Create components for an entity
 * PlayerArchetype::Create("player1");
 * 
 * // Get a specific component
 * auto* position = PlayerArchetype::GetComponent<PositionComponent>("player1");
 * 
 * // Destroy all components
 * PlayerArchetype::DestroyFor("player1");
 */

} // namespace entities 