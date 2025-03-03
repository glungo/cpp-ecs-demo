#include <cassert>
#include "include/archetype.h"

namespace entities {
namespace tests {

DEFINE_COMPONENT(PositionComponent, 10)
    COMPONENT_MEMBER(float, x) = 0.0f;
    COMPONENT_MEMBER(float, y) = 0.0f;
    COMPONENT_MEMBER(float, z) = 0.0f;
END_COMPONENT

DEFINE_ARCHETYPE(MovableEntity, PositionComponent);

void test_archetype_entity_iteration() {
    // Create multiple entities
    for (int i = 0; i < 3; i++) {
        MovableEntity::Create("entity_" + std::to_string(i));
    }

    // Test getting all entities with a component
    const auto& entities = MovableEntity::GetEntities<PositionComponent>();
    assert(entities.size() == 3 && "Should have 3 entities with position component");

    // Test getting all components
    const auto& components = MovableEntity::GetComponents<PositionComponent>();
    assert(components.size() == 3 && "Should have 3 position components");

    // Cleanup
    for (int i = 0; i < 3; i++) {
        MovableEntity::DestroyFor("entity_" + std::to_string(i));
    }
}

} // namespace tests
} // namespace entities

int main() {
    entities::tests::test_archetype_entity_iteration();
    return 0;
} 