#include <cassert>
#include "include/entity_manager.h"

namespace entities {
namespace tests {

void test_basic_entity_creation() {
    EntityManager manager(3);
    
    auto* entity1 = manager.CreateEntity();
    assert(entity1 != nullptr && "Entity 1 should be created");
    assert(manager.IsEntityActive(entity1) && "Entity 1 should be active");

    auto* entity2 = manager.CreateEntity();
    assert(entity2 != nullptr && "Entity 2 should be created");
    assert(manager.IsEntityActive(entity2) && "Entity 2 should be active");

    manager.DestroyEntity(entity1);
    manager.DestroyEntity(entity2);
}

} // namespace tests
} // namespace entities

int main() {
    entities::tests::test_basic_entity_creation();
    return 0;
} 