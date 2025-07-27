#include <cassert>
#include "include/archetype.h"

namespace entities {
namespace tests {

DEFINE_COMPONENT(PositionComponent, 10)
    COMPONENT_MEMBER(float, x) = 0.0f;
    COMPONENT_MEMBER(float, y) = 0.0f;
    COMPONENT_MEMBER(float, z) = 0.0f;
END_COMPONENT

DEFINE_COMPONENT(VelocityComponent, 10)
    COMPONENT_MEMBER(float, vx) = 0.0f;
    COMPONENT_MEMBER(float, vy) = 0.0f;
    COMPONENT_MEMBER(float, vz) = 0.0f;
END_COMPONENT

DEFINE_ARCHETYPE(MovableEntity,
    PositionComponent,
    VelocityComponent
);

void test_archetype_creation() {
    std::string entity_id = "test_entity";
    MovableEntity::Create(entity_id);

    auto* pos = MovableEntity::GetComponent<PositionComponent>(entity_id);
    auto* vel = MovableEntity::GetComponent<VelocityComponent>(entity_id);

    assert(pos != nullptr && "Should have position component");
    assert(vel != nullptr && "Should have velocity component");

    MovableEntity::DestroyFor(entity_id);
}

}
} // namespace entities::tests

int main() {
    entities::tests::test_archetype_creation();
    return 0;
} 
