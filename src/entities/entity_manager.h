#pragma once
#include <array>
#include "entity.h"
#include <vector>
#include <memory>
#include <map>
#include "../utils/uuid.h"
#include "pool.h"

namespace entities {

class EntityManager {
public:
    static constexpr size_t DEFAULT_POOL_SIZE = 10000;
    static constexpr size_t INVALID_INDEX = -1;

    EntityManager(size_t pool_size = DEFAULT_POOL_SIZE) : entity_pool(pool_size) {}

    Entity* CreateEntity() {
        return entity_pool.Create(uuid::generate_uuid());
    }

    void DestroyEntity(Entity* entity) {
        entity_pool.Destroy(entity);
    }

    bool IsEntityActive(Entity* entity) {
        return entity_pool.IsActive(entity);
    }

    size_t GetEntityIndex(Entity* entity) {
        for (size_t i = 0; i < entity_pool.GetActiveCount(); i++) {
            if (entity_pool.Get(i) == entity) {
                return i;
            }
        }
        return INVALID_INDEX;
    }   

    size_t GetActiveCount() const {
        return entity_pool.GetActiveCount();
    }

    void ClearPool() {
        entity_pool.Clear();
    }

private:
    Pool<Entity> entity_pool;
};

} // namespace entities 