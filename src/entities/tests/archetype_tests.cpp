#include <cassert>
#include <iostream>
#include "../archetype.h"

using namespace entities;

// Define test components
DEFINE_COMPONENT(PositionComponent, 100)
    COMPONENT_MEMBER(float, x) = 0.0f;
    COMPONENT_MEMBER(float, y) = 0.0f;
    COMPONENT_MEMBER(float, z) = 0.0f;
END_COMPONENT

DEFINE_COMPONENT(VelocityComponent, 100)
    COMPONENT_MEMBER(float, vx) = 0.0f;
    COMPONENT_MEMBER(float, vy) = 0.0f;
    COMPONENT_MEMBER(float, vz) = 0.0f;
END_COMPONENT

// Define test archetype
DEFINE_ARCHETYPE(MovableEntity,
    PositionComponent
);

DEFINE_ARCHETYPE(MovableEntity2,
    PositionComponent,
    VelocityComponent
); 

// Test creating and destroying a small number of entities
void test_archetype_creation() {
    std::string entity_id = "test_entity";
    MovableEntity2::Create(entity_id);

    auto entities = MovableEntity2::GetEntities<PositionComponent>();
    assert(entities.size() == 1 && "Should have 1 entity");

    auto* pos = MovableEntity2::GetComponent<PositionComponent>(entity_id);
    assert(pos != nullptr && "Should have position component");

    auto* vel = MovableEntity2::GetComponent<VelocityComponent>(entity_id);
    assert(vel != nullptr && "Should have velocity component");

    MovableEntity2::DestroyFor(entity_id);
}

// Test getting components
void test_get_component() {
    std::string entity_id = "test_entity";
    MovableEntity::Create(entity_id);
    std::string entity_id2 = "test_entity2";
    MovableEntity::Create(entity_id2);

    const std::vector<PositionComponent*> pos_components = MovableEntity::GetComponents<PositionComponent>();
    assert(pos_components.size() == 2 && "Should have 2 position components");

    MovableEntity::DestroyFor(entity_id);
    MovableEntity::DestroyFor(entity_id2); 
}

// Test creating and destroying many entities
void test_component_pool_fragmentation() {
    // Create a smaller number of entities for testing
    const int num_entities = 20;
    const int num_to_destroy = 5;
    const int num_to_create_after = 3;
    
    std::cout << "Creating " << num_entities << " entities" << std::endl;
    for (int i = 0; i < num_entities; i++) {
        MovableEntity::Create(std::to_string(i));
    }
    
    std::cout << "Destroying " << num_to_destroy << " entities" << std::endl;
    for (int i = 0; i < num_to_destroy; i++) {
        MovableEntity::DestroyFor(std::to_string(i));
    }
    
    std::cout << "Creating " << num_to_create_after << " more entities" << std::endl;
    for (int i = 0; i < num_to_create_after; i++) {
        MovableEntity::Create(std::to_string(i + 100));  // Use different IDs
    }
    
    // Get all position components
    auto pos_components = MovableEntity::GetComponents<PositionComponent>();
    
    // Check that we have the expected number of components
    const int expected_count = num_entities - num_to_destroy + num_to_create_after;
    assert(pos_components.size() == expected_count && 
           "Should have correct number of position components");
    
    // Check that all components are active
    std::cout << "Checking that all components are active" << std::endl;
    size_t i = 0;
    for (auto* comp : pos_components) {
        assert(comp != nullptr && "Component should not be null");
        assert(PositionComponent::IsActive(comp) && "Component should be active");
        i++;
    }
    assert(i == expected_count && "Should have checked all components");
}

int main() {
    test_archetype_creation();
    test_get_component();
    test_component_pool_fragmentation();
    return 0;
}