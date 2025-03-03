#include <cassert>
#include <iostream>
#include "../component.h"
using namespace entities;
// Define a test component

DEFINE_COMPONENT(TestComponent, 100)
    COMPONENT_MEMBER(float, x) = 0.0f;
END_COMPONENT

void test_find_owner() {
    auto* comp = TestComponent::Create();
    TestComponent::RegisterOwner("test_entity", comp);
    
    const std::string* owner = TestComponent::FindOwnerEntity(comp);
    assert(owner != nullptr && "Owner should not be null");
    assert(*owner == "test_entity" && "Owner should be 'test_entity'");
    
    TestComponent::Destroy(comp);
}

int main() {
    test_find_owner();
    return 0;
} 