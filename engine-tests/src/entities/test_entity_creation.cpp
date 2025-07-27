#include <cassert>
#include <iostream>
#include "include/entity.h"

namespace entities {
namespace tests {

int test_entity_creation() {
    std::cout << "=== Testing Entity Creation ===" << std::endl;
    
    try {
        Entity entity("test_entity");
        assert(entity.m_uuid == "test_entity");
        std::cout << "Entity creation test passed!" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during entity creation test: " << e.what() << std::endl;
        return 1;
    }
}

}
} // namespace entities

int main() {
    return entities::tests::test_entity_creation();
}