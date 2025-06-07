#pragma once

namespace entities {

class EntityDebugger {
public:
    EntityDebugger();
    ~EntityDebugger();

    void DisplayEntityMemoryState();
    void DisplayComponentMemoryState();
};

} // namespace entities