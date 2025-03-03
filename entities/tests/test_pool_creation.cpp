#include <cassert>
#include "include/pool.h"

using namespace entities;

struct TestItem {
    int value;
    TestItem(int v) : value(v) {}
};

void test_pool_creation() {
    Pool<TestItem> pool(10);
    auto* item = pool.Create(42);
    assert(item != nullptr);
    assert(item->value == 42);
    assert(pool.IsActive(item));
    pool.Destroy(item);
    assert(!pool.IsActive(item));
}

int main() {
    test_pool_creation();
    return 0;
} 