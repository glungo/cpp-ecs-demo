#include <iostream>
#include "entities/entity.h"
#include "entities/entity_manager.h"

constexpr size_t entity_memory_pool_size = sizeof(Entity) * entities::EntityManager::DEFAULT_POOL_SIZE;

int main(int, char**){
    //Entity memory pool
    Entity* entity_memory_pool = static_cast<Entity*>(malloc(entity_memory_pool_size));

    //EntityManager
    entities::EntityManager world(entities::EntityManager::DEFAULT_POOL_SIZE);
}
