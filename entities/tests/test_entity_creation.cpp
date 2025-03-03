#include <cassert>
#include "include/entity.h"

void test_entity_creation() {
    Entity entity("test_entity");
    assert(entity.m_uuid == "test_entity");
}

int main() {
    test_entity_creation();
    return 0;
} 