#pragma once
#include <array>
#include "entity.h"
#include <vector>
#include <memory>
#include <map>
#include "utils/uuid.h"
#include "pool.h"
#include "utils/singleton.h"

namespace entities {

class EntityManager : protected Pool<Entity>, public Singleton<EntityManager> {
    DECLARE_SINGLETON(EntityManager)
public:
//TODO: move all constants to a constants header
    static constexpr size_t DEFAULT_POOL_SIZE = 10000;
    static constexpr size_t INVALID_INDEX = -1;
#ifdef ENTITIES_DEBUG
    EntityManager(size_t pool_size = DEFAULT_POOL_SIZE) : Pool<Entity>(pool_size), Singleton<EntityManager>() {}
#endif
    
    // Expose specific methods from Pool<Entity>
    using Pool<Entity>::IsActive;
    using Pool<Entity>::Get;
    using Pool<Entity>::GetActiveCount;
    using Pool<Entity>::Clear;
    using Pool<Entity>::GetPtr;
    using Pool<Entity>::GetAll;
    using Pool<Entity>::Destroy;

    // Custom methods that may need special handling
    Entity* CreateEntity() {
        return Create(uuid::generate_uuid());
    }

    size_t GetEntityIndex(Entity* entity) {
        for (size_t i = 0; i < GetActiveCount(); i++) {
            if (Get(i) == entity) {
                return i;
            }
        }
        return INVALID_INDEX;
    }
protected:
#ifndef ENTITIES_DEBUG
    EntityManager(size_t pool_size = DEFAULT_POOL_SIZE) : Pool<Entity>(pool_size), Singleton<EntityManager>() {}
#endif
};

} // namespace entities 