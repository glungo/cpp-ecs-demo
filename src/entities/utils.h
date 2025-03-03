#pragma once
#include <vector>
#include <string>
#include <map>
#include <set>
#include "../utils/container_utils.h"

namespace entities {

// Forward declarations
template<typename T> class Component;
template<typename... Components> class Archetype;
class Entity;  // Add this forward declaration

namespace utils {

// Component-related utilities
namespace component {
    // Get all components of a specific type
    template<typename T>
    std::vector<T*> GetComponents() {
        return T::GetAllComponents();
    }
    
    // Get component for an entity
    template<typename T>
    T* GetComponent(const std::string& entity_id) {
        return T::GetComponentForEntity(entity_id);
    }
}

// Archetype-related utilities
namespace archetype {
    // Get all entities with a specific component type
    template<typename A, typename T>
    std::vector<std::string> GetEntities() {
        return A::template GetEntities<T>();
    }
    
    // Get all components of a specific type from an archetype
    template<typename A, typename T>
    std::vector<T*> GetComponents() {
        return A::template GetComponents<T>();
    }

    // Get the owner of a component
    template<typename T>
    Entity* GetOwner(T* component) {
        //search in the maps for the component
        for (auto& pair : Archetype<T>::component_maps) {
            if (pair.second == component) {
                return pair.first;
            }
        }
        return nullptr;
    }
}

// Convert a set to a vector
template<typename T>
std::vector<T> SetToVector(const std::set<T>& set) {
    return std::vector<T>(set.begin(), set.end());
}

// Extract values from a map into a vector
template<typename K, typename V>
std::vector<V> MapValuesToVector(const std::map<K, V>& map) {
    std::vector<V> result;
    result.reserve(map.size());
    for (const auto& pair : map) {
        result.push_back(pair.second);
    }
    return result;
}

// Find the entity ID that owns a specific component
template<typename T>
const std::string* FindEntityWithComponent(const T* component) {
    if (!component) return nullptr;
    
    // Get the component map from the component's static storage
    const auto& component_map = T::component_map;
    
    // Search for the component in the map
    for (const auto& pair : component_map) {
        if (pair.second == component) {
            return &pair.first;
        }
    }
    return nullptr;
}

// Overload for non-const components
template<typename T>
const std::string* FindEntityWithComponent(T* component) {
    return FindEntityWithComponent(static_cast<const T*>(component));
}

} // namespace utils
} // namespace entities 