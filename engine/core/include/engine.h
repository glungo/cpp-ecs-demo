#pragma once

#include "entity_manager.h"
#include "job_scheduler.h"
#include <memory>

namespace engine {

class Engine {
public:
    Engine();
    ~Engine();

    // Initialize the engine
    bool initialize();
    
    // Shutdown the engine
    void shutdown();
    
    // Run a single frame of the engine
    void update(float deltaTime);
    
    //TODO: handle the entity manager memory on engine instead of using a singleton
    // Get entity manager
    entities::EntityManager& getEntityManager() { return entities::EntityManager::getInstance(); }
    
    // Get job scheduler
    JobSystem::JobScheduler& getJobScheduler() { return *m_jobScheduler; }

private:
    std::unique_ptr<JobSystem::JobScheduler> m_jobScheduler;
    bool m_initialized;
};

} // namespace engine