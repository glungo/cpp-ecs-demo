#include "../entity_manager.h"
#include <cassert>  
#include <iostream>

using namespace entities;

void test_basic_entity_creation(EntityManager& manager) {
    // Test getting first entity
    auto* entity1 = manager.CreateEntity();
    assert(entity1 != nullptr && "Entity 1 should be created");
    assert(manager.IsEntityActive(entity1) == true && "Entity 0 should be active");

    auto* entity2 = manager.CreateEntity();
    assert(entity2 != nullptr && "Entity 2 should be created");
    assert(manager.IsEntityActive(entity2) == true && "Entity 1 should be active");

    //cleanup
    manager.DestroyEntity(entity1);
    manager.DestroyEntity(entity2);
}

void test_pool_overflow(EntityManager& manager) {
    // Fill the pool
    for (int i = 0; i < 3; i++) {
        auto* entity = manager.CreateEntity();
        assert(entity != nullptr && "Entity should be created");
        assert(manager.IsEntityActive(entity) == true && "Entity should be active");
    }

    // Try to overflow
    auto* entity4 = manager.CreateEntity();
    assert(entity4 == nullptr && "Should return nullptr when pool is full");

    //cleanup
    manager.ClearPool();
}

void test_entity_reuse(EntityManager& manager) {
    // Create 2 entities
    auto* entity1 = manager.CreateEntity();
    auto* entity2 = manager.CreateEntity();
    
    assert(entity1 != nullptr && "Entity 1 should be created");
    assert(entity2 != nullptr && "Entity 2 should be created");
    assert(manager.GetActiveCount() == 2 && "Should have 2 active entities");
    
    // Destroy the first entity
    manager.DestroyEntity(entity1);
    
    // Verify we now have 1 active entity
    assert(manager.GetActiveCount() == 1 && "Entity count should be 1 after destroying entity1");
    assert(manager.IsEntityActive(entity1) == false && "Entity 1 should be inactive");
    assert(manager.IsEntityActive(entity2) == true && "Entity 2 should still be active");

    // Create a new entity, which should reuse the slot
    auto* new_entity = manager.CreateEntity();
    assert(new_entity != nullptr && "New entity should be created");
    
    // Verify the new entity is active
    assert(manager.IsEntityActive(new_entity) == true && "New entity should be active");
    
    // Verify we now have 2 active entities again
    assert(manager.GetActiveCount() == 2 && "Entity count should be 2 after creating new entity");

    //cleanup
    manager.ClearPool();
}

void run_all_tests() {
    EntityManager manager(3);  // Create manager with pool size of 3 entities

    test_basic_entity_creation(manager);
    test_pool_overflow(manager);
    test_entity_reuse(manager);
}

int main() {
    run_all_tests();
    return 0;
} 