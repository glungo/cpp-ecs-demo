#include <cassert>
#include "include/entity_manager.h"

namespace entities {
namespace tests {

void test_entity_reuse() {
    EntityManager manager(3);
    
    // Create 2 entities
    auto* entity1 = manager.CreateEntity();
    auto* entity2 = manager.CreateEntity();
    
    assert(entity1 != nullptr && "Entity 1 should be created");
    assert(entity2 != nullptr && "Entity 2 should be created");
    assert(manager.GetActiveCount() == 2 && "Should have 2 active entities");
    
    // Destroy the first entity
    manager.Destroy(entity1);
    
    // Verify we now have 1 active entity
    assert(manager.GetActiveCount() == 1 && "Entity count should be 1 after destroying entity1");
    assert(!manager.IsActive(entity1) && "Entity 1 should be inactive");
    assert(manager.IsActive(entity2) && "Entity 2 should still be active");

    // Create a new entity, which should reuse the slot
    auto* new_entity = manager.CreateEntity();
    assert(new_entity != nullptr && "New entity should be created");
    assert(manager.IsActive(new_entity) && "New entity should be active");
    assert(manager.GetActiveCount() == 2 && "Entity count should be 2 after creating new entity");

    manager.Clear();
}

} // namespace tests
} // namespace entities

int main() {
    entities::tests::test_entity_reuse();
    return 0;
} 