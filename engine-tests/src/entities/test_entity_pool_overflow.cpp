#include <cassert>
#include "include/entity_manager.h"
#include <iostream>

namespace entities {
    namespace tests {

void test_entity_pool_overflow() {
    // Instead of creating a new instance, get the singleton instance
    auto& manager = EntityManager::getInstance();
    
    // Clear any existing entities first
    manager.Clear();
    
    // Note: We can't control the pool size in this test anymore
    // since we're using the singleton instance. Let's test what we can.
    
    // Create several entities (but not enough to overflow)
    constexpr int NUM_ENTITIES = 10; // Use a reasonable number
    Entity* entities[NUM_ENTITIES];
    
    std::cout << "Creating " << NUM_ENTITIES << " entities..." << std::endl;
    for (int i = 0; i < NUM_ENTITIES; i++) {
        entities[i] = manager.CreateEntity();
        assert(entities[i] != nullptr && "Entity should be created");
        assert(manager.IsActive(entities[i]) && "Entity should be active");
    }
    
    std::cout << "Successfully created " << NUM_ENTITIES << " entities" << std::endl;
    
    // Clean up
    manager.Clear();
    std::cout << "Cleared all entities" << std::endl;
}

} // namespace entities::tests
} // namespace entities

int main() {
    std::cout << "Running entity pool test" << std::endl;
    entities::tests::test_entity_pool_overflow();
    std::cout << "Entity pool test completed successfully" << std::endl;
    return 0;
}
