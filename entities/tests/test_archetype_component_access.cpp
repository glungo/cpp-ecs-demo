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

void test_archetype_component_access() {
    std::string entity_id = "test_entity";
    MovableEntity::Create(entity_id);

    auto* pos = MovableEntity::GetComponent<PositionComponent>(entity_id);
    assert(pos != nullptr && "Should have position component");

    // Test component value access
    pos->x = 1.0f;
    pos->y = 2.0f;
    pos->z = 3.0f;

    auto* retrieved_pos = MovableEntity::GetComponent<PositionComponent>(entity_id);
    assert(retrieved_pos->x == 1.0f && "Component value should persist");
    assert(retrieved_pos->y == 2.0f && "Component value should persist");
    assert(retrieved_pos->z == 3.0f && "Component value should persist");

    MovableEntity::DestroyFor(entity_id);
}

} // namespace tests
} // namespace entities

int main() {
    entities::tests::test_archetype_component_access();
    return 0;
} 