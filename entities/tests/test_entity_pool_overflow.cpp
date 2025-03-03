#include <cassert>
#include "include/entity_manager.h"

namespace entities {
namespace tests {

void test_entity_pool_overflow() {
    EntityManager manager(3);
    
    // Fill the pool
    for (int i = 0; i < 3; i++) {
        auto* entity = manager.CreateEntity();
        assert(entity != nullptr && "Entity should be created");
        assert(manager.IsEntityActive(entity) && "Entity should be active");
    }

    // Try to overflow
    auto* entity4 = manager.CreateEntity();
    assert(entity4 == nullptr && "Should return nullptr when pool is full");

    manager.ClearPool();
}

} // namespace tests
} // namespace entities

int main() {
    entities::tests::test_entity_pool_overflow();
    return 0;
} 