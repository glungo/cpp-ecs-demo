#pragma once

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>
#include <map>
#include "pool.h"

namespace entities {

template<typename T>
class Component {
public:
    virtual ~Component() = default;
    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;
    Component(Component&&) = default;
    Component& operator=(Component&&) = default;

protected:
    Component() = default;
};

} // namespace entities

namespace detail {

// Type trait to check if T is a valid component member type
template<typename T>
struct is_valid_component_member {
    static constexpr bool value = std::disjunction_v<
        std::is_same<T, bool>,
        std::is_same<T, int>,
        std::is_same<T, float>,
        std::is_same<T, std::string>
    >;
};

// Helper variable template
template<typename T>
inline constexpr bool is_valid_component_member_v = is_valid_component_member<T>::value;

// Base class for validated members
struct ValidatedMember {};

// Template for type-safe members
template<typename T>
struct Member : ValidatedMember {
    static_assert(is_valid_component_member<T>::value, 
        "Member type must be bool, int, float, or std::string");
    T value;

    Member() = default;
    Member(const T& v) : value(v) {}
    Member(T&& v) : value(std::move(v)) {}

    operator T&() { return value; }
    operator const T&() const { return value; }
};

// Helper to check if all data members of a class are ValidatedMembers
template<typename T>
struct has_only_validated_members {
private:
    template<typename U>
    static constexpr bool check_member(U T::*) {
        // Only check data members, ignore member functions
        if constexpr (std::is_member_function_pointer_v<U T::*>) {
            return true;
        }
        return std::is_base_of_v<ValidatedMember, U>;
    }

    template<typename... Args>
    static constexpr bool check_all(Args... args) {
        return (check_member(args) && ...);
    }

    template<typename U>
    static auto test(U*) -> decltype(
        check_all(&U::__host_member_begin, &U::__host_member_end),
        std::true_type{}
    );

    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test(static_cast<T*>(nullptr)))::value;
};

} // namespace detail

#define COMPONENT_MEMBER(Type, Name) \
    detail::Member<Type> Name

#define COMPONENT_MEMBER_DEFAULT(Type, Name, Default) detail::Member<Type> Name = Default

#define DEFINE_COMPONENT(Name, PoolSize) \
    struct Name : public Component<Name> { \
        static inline entities::Pool<Name> pool{PoolSize}; \
        static inline std::map<std::string, Name*> component_map; \
        static Name* Create() { return pool.Create(); } \
        template<typename... Args> \
        static Name* Create(Args&&... args) { return pool.Create(std::forward<Args>(args)...); } \
        static void Destroy(Name* component) { \
            const std::string* owner = FindOwnerEntity(component); \
            if (owner) { \
                component_map.erase(*owner); \
            } \
            pool.Destroy(component); \
        } \
        static bool IsActive(Name* component) { return pool.IsActive(component); } \
        static size_t GetActiveCount() { return pool.GetActiveCount(); } \
        static Name* GetComponentsPtr() { return pool.GetPtr(); } \
        static std::vector<Name*> GetAll() { return pool.GetAll(); } \
        static const std::string* FindOwnerEntity(const Name* component) { \
            if (!component) return nullptr; \
            for (const auto& pair : component_map) { \
                if (pair.second == component) { \
                    return &pair.first; \
                } \
            } \
            return nullptr; \
        } \
        static const std::string* FindOwnerEntity(Name* component) { \
            return FindOwnerEntity(static_cast<const Name*>(component)); \
        } \
        static void RegisterOwner(const std::string& entity_id, Name* component) { \
            if (component) { \
                component_map[entity_id] = component; \
            } \
        } \
        static void UnregisterOwner(const std::string& entity_id) { \
            component_map.erase(entity_id); \
        }

#define END_COMPONENT };

/*
 * Example usage:
 * 
 * DEFINE_COMPONENT(PositionComponent, 1000)
 *     COMPONENT_MEMBER(float, x) = 0.0f;
 *     COMPONENT_MEMBER(float, y) = 0.0f;
 *     COMPONENT_MEMBER(float, z) = 0.0f;
 * END_COMPONENT
 * 
 * DEFINE_COMPONENT(PlayerComponent)
 *     COMPONENT_MEMBER(std::string, name);
 *     COMPONENT_MEMBER_DEFAULT(int, level, 1);
 *     COMPONENT_MEMBER_DEFAULT(float, health, 100.0f);
 *     COMPONENT_MEMBER_DEFAULT(bool, isActive, true);
 * END_COMPONENT
 */ 