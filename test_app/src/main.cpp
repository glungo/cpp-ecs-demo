#include <iostream>
#include "include/entity.h"
#include "include/entity_manager.h"

constexpr size_t entity_memory_pool_size = sizeof(Entity) * entities::EntityManager::DEFAULT_POOL_SIZE;

int main(int, char**){
    //EntityManager
    std::cout << "Initializing EntityManager" << std::endl;
    entities::EntityManager EntityManager(entities::EntityManager::DEFAULT_POOL_SIZE);
}
