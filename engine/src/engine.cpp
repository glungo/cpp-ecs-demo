#include "engine/engine.h"
#include "entities/include/utils/logger.h"
#include "entities/include/utils/LogMacros.h"

namespace engine {

Engine::Engine() 
    : m_initialized(false) 
{
}

Engine::~Engine() {
    if (m_initialized) {
        shutdown();
    }
}

bool Engine::initialize() {
    if (m_initialized) {
        LOG_WARNING << "Engine is already initialized" << LOG_END;
        return true;
    }

    LOG_INFO << "Initializing engine" << LOG_END;
    
    // The JobScheduler doesn't have an initialize method, it's initialized in the constructor
    
    // EntityManager is a singleton and doesn't have an initialize method
    
    m_initialized = true;
    LOG_INFO << "Engine initialized successfully" << LOG_END;
    return true;
}

void Engine::shutdown() {
    if (!m_initialized) {
        LOG_WARNING << "Engine is not initialized" << LOG_END;
        return;
    }
    
    LOG_INFO << "Shutting down engine" << LOG_END;
    
    // No explicit shutdown needed as both classes will clean up in their destructors
    
    m_initialized = false;
    LOG_INFO << "Engine shutdown completed" << LOG_END;
}

void Engine::update(float deltaTime) {
    if (!m_initialized) {
        LOG_ERROR << "Cannot update: Engine is not initialized" << LOG_END;
        return;
    }
    
    // Process jobs
    m_jobScheduler.Update(deltaTime);
    
    // No explicit update method for EntityManager in the current implementation
}

} // namespace engine