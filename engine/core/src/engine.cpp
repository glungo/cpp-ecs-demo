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
    if (m_initialized) {
        LOG_WARNING << "Engine is already initialized" << LOG_END;
        return true;
    }

    m_window = std::make_shared<glfw_window>();
    if (!m_window) {
        LOG_ERROR << "Failed to create window" << LOG_END;
        return false;
    }

    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->initialize(*m_window)) {
        LOG_ERROR << "Failed to initialize renderer" << LOG_END;
        return false;
    }
	
    // Connect window resize callback to rendering context
    m_window->setResizeCallback([this](int width, int height) {
        if (m_renderer) {
            m_renderer->handleResize(width, height);
        }
    });

    m_jobScheduler = std::make_unique<JobSystem::JobScheduler>();
    if (!m_jobScheduler) {
        LOG_ERROR << "Failed to create JobScheduler" << LOG_END;
        return false;
    }

    while (!m_window->shouldClose()) {
		m_renderer->render();
        m_window->pollEvents();
    }
	m_window->shutdown();

    LOG_INFO << "Engine initialized successfully" << LOG_END;
    m_initialized = true;
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
    m_renderer->render();
}

} // namespace engine