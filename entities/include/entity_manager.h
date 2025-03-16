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

class EntityManager : public Singleton<EntityManager> {
public:
    static constexpr size_t DEFAULT_POOL_SIZE = 10000;
    static constexpr size_t INVALID_INDEX = -1;

    EntityManager(size_t pool_size = DEFAULT_POOL_SIZE) : entity_pool(pool_size), Singleton<EntityManager>() {}
    
    Entity* CreateEntity() {
        return entity_pool.Create(uuid::generate_uuid());
    }

    std::vector<Entity*> GetActiveEntities() const{
        return entity_pool.GetAll();
    }

    const Entity* GetPoolPtr() const {
        return entity_pool.GetPtr();
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