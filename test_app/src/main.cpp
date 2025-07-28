#include <iostream>
#include "engine.h"

int main() {
    engine::Engine engine;
    if (!engine.initialize()) {
        std::cerr << "Failed to initialize the engine" << std::endl;
        return 1;
    }
    return 0;
}
