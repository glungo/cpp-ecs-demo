#ifndef ENGINE_H
#define ENGINE_H

#include "entities/include/entity_manager.h"
#include "entities/include/job_scheduler.h"

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
    
    // Get entity manager
    entities::EntityManager& getEntityManager() { return entities::EntityManager::getInstance(); }
    
    // Get job scheduler
    JobSystem::JobScheduler& getJobScheduler() { return m_jobScheduler; }
    
private:
    JobSystem::JobScheduler m_jobScheduler;
    bool m_initialized;
};

} // namespace engine

#endif // ENGINE_H