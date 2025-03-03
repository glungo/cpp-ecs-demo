#include <cassert>
#include "include/component.h"

namespace entities {
namespace tests {

DEFINE_COMPONENT(TestComponent, 10)
    COMPONENT_MEMBER(float, x) = 0.0f;
    COMPONENT_MEMBER(float, y) = 0.0f;
END_COMPONENT

void test_component_creation() {
    auto* comp = TestComponent::Create();
    assert(comp != nullptr && "Component should be created");
    assert(TestComponent::IsActive(comp) && "Component should be active");
    TestComponent::Destroy(comp);
}

} // namespace tests
} // namespace entities

int main() {
    entities::tests::test_component_creation();
    return 0;
} 