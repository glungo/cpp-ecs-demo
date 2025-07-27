#include <cassert>
#include "include/entity_manager.h"
#include <iostream>

namespace entities {
    namespace tests {

void test_entity_reuse() {
    // Instead of creating a new instance, get the singleton instance
    auto& manager = EntityManager::getInstance();
    
    // Clear any existing entities first
    manager.Clear();
    
    std::cout << "Creating entities for reuse test..." << std::endl;
    
    // Create 2 entities
    auto* entity1 = manager.CreateEntity();
    auto* entity2 = manager.CreateEntity();
    
    assert(entity1 != nullptr && "Entity 1 should be created");
    assert(entity2 != nullptr && "Entity 2 should be created");
    assert(manager.GetActiveCount() == 2 && "Should have 2 active entities");
    
    std::cout << "Created 2 entities successfully" << std::endl;
    
    // Destroy the first entity
    manager.Destroy(entity1);
    
    std::cout << "Destroyed entity 1, now testing reuse..." << std::endl;
    
    // Verify we now have 1 active entity
    assert(manager.GetActiveCount() == 1 && "Entity count should be 1 after destroying entity1");
    assert(!manager.IsActive(entity1) && "Entity 1 should be inactive");
    assert(manager.IsActive(entity2) && "Entity 2 should still be active");

    // Create a new entity, which should reuse the slot
    auto* new_entity = manager.CreateEntity();
    assert(new_entity != nullptr && "New entity should be created");
    assert(manager.IsActive(new_entity) && "New entity should be active");
    assert(manager.GetActiveCount() == 2 && "Entity count should be 2 after creating new entity");
    
    std::cout << "Successfully created new entity that reused the slot" << std::endl;

    manager.Clear();
    std::cout << "Cleared all entities" << std::endl;
}

    } // namespace tests

} // namespace entities

int main() {
    std::cout << "Running entity reuse test" << std::endl;
    entities::tests::test_entity_reuse();
    std::cout << "Entity reuse test completed successfully" << std::endl;
    return 0;
}
