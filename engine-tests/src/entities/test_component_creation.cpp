#include <cassert>
#include <iostream>
#include "include/component.h"

namespace entities {
    namespace tests {

DEFINE_COMPONENT(TestComponent, 10)
    COMPONENT_MEMBER(float, x) = 0.0f;
    COMPONENT_MEMBER(float, y) = 0.0f;
END_COMPONENT

void test_component_creation_impl() {
    auto* comp = TestComponent::Create();
    assert(comp != nullptr && "Component should be created");
    assert(TestComponent::IsActive(comp) && "Component should be active");
    TestComponent::Destroy(comp);
}

} // namespace tests
} // namespace entities

int main() {
    std::cout << "Running component creation test..." << std::endl;
    entities::tests::test_component_creation_impl();
    std::cout << "Component creation test passed!" << std::endl;
    return 0;
} 
