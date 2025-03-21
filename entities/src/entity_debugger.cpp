#include "include/debugger/entity_debugger.h"
#include "include/entity_manager.h"

namespace entities {

void EntityDebugger::DisplayEntityMemoryState() {
    // Display the memory state of the entity
    EntityManager& entityManager = EntityManager::getInstance();
    LOG() << "Entity Memory State: Active Count: " << entityManager.GetActiveCount() << LOG_END;
    std::map<size_t, std::string> entity_addresses;
    for (auto entity : entityManager.GetActiveEntities())
    {
        entity_addresses[reinterpret_cast<size_t>(entity)] = entity->m_uuid;
    }
    //print the memory blocks between each entity
    //  ------------
    // | <EntityId> |
    //  ------------
    for (auto it = entity_addresses.begin(); it != entity_addresses.end(); it++)
    {
        LOG() << "  --" << it->first << "--" << LOG_END;
        LOG() << " | " << it->second << " |" << LOG_END;
        LOG() << "  ------------" << LOG_END;
    }
}

void EntityDebugger::DisplayComponentMemoryState() {
    // Display the memory state of the components
}
} // namespace entities
