#include "engine/engine.h"
#include "utils/logger.h"
#include "utils/LogMacros.h"

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
    m_jobScheduler = std::make_unique<JobSystem::JobScheduler>();
    if (!m_jobScheduler) {
        LOG_ERROR << "Failed to create JobScheduler" << LOG_END;
        return false;
    }
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
    m_jobScheduler->Update(deltaTime);
}

} // namespace engine