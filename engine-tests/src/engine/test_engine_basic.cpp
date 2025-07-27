#include "engine.h"
#include <cassert>
#include <iostream>

namespace engine {
    namespace tests {

// Simple component for testing
struct TestComponent {
    int value;
    TestComponent(int v = 0) : value(v) {}
};

void test_engine_basic() {
    std::cout << "=== Testing Engine Basic Functionality ===" << std::endl;
    
    // Create engine instance
    Engine engine;
    
    // Initialize engine
    bool initResult = engine.initialize();
    assert(initResult && "Engine initialization should succeed");
    std::cout << "Engine initialized successfully" << std::endl;
    
    // Create an entity
    auto& entityManager = engine.getEntityManager();
    auto entity = entityManager.CreateEntity(); // Fixed: capital C in CreateEntity
    assert(entity != nullptr && "Entity creation should succeed");
    
    // For now, we'll just verify the entity was created
    // The component functionality will need to be implemented in EntityManager
    std::cout << "Entity created successfully" << std::endl;
    
    // Run a few update frames
    for (int i = 0; i < 3; i++) {
        engine.update(1.0f / 60.0f);
        std::cout << "Engine update frame: " << i << std::endl;
    }
    
    // Shutdown the engine
    engine.shutdown();
    std::cout << "Engine shutdown successful" << std::endl;
    
    std::cout << "All engine tests passed!" << std::endl;
}

}
} // namespace engine::tests

int main() {
    engine::tests::test_engine_basic();
    std::cout << "Engine basic test completed successfully." << std::endl;
    return 0;
}
