#include "engine.h"
#include "logger.h"
#include "LogMacros.h"

#include "window.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

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
    m_window = std::make_shared<glfw_window>();
    if (!m_window) {
        LOG_ERROR << "Failed to create window" << LOG_END;
        return false;
    }

    if (m_initialized) {
        LOG_WARNING << "Engine is already initialized" << LOG_END;
        return true;
    }
    m_jobScheduler = std::make_unique<JobSystem::JobScheduler>();
    if (!m_jobScheduler) {
        LOG_ERROR << "Failed to create JobScheduler" << LOG_END;
        return false;
    }
    
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
        nullptr);
    LOG_INFO << extensionCount << " extensions supported\n";  
    glm::mat4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;

    while (!m_window->shouldClose()) {
        m_window->pollEvents();
    }
	m_window->shutdown();

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