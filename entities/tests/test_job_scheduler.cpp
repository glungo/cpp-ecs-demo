#include "entities/include/job_scheduler.h"
#include "entities/include/system.h"
#include <cassert>
#include <iostream>
#include "entities/include/component.h"
using namespace entities;
namespace JobSystem {

DEFINE_COMPONENT(MockComponent, 100)
    COMPONENT_MEMBER_DEFAULT(int, m_value, 0);
END_COMPONENT

DEFINE_COMPONENT(MockComponent2, 100)
    COMPONENT_MEMBER_DEFAULT(int, m_value2, 0);
END_COMPONENT

// Mock job class for testing
class MockJob : public Job<MockComponent> {
public:
    MockJob() : Job<MockComponent>("MockJob", 
        [](float dt, const std::vector<std::tuple<MockComponent*>>& components) {
            for (auto& component : components) {
                std::cout << "MockJob: " << std::get<0>(component)->m_value << std::endl;
                std::get<0>(component)->m_value += 1;
                std::cout << "MockJob: " << std::get<0>(component)->m_value << std::endl;
            }
        }
    ) {}
    
    virtual void RefreshCache() override {
        auto components = MockComponent::GetAll();
        m_cache = std::vector<std::tuple<MockComponent*>>(components.begin(), components.end());
        std::cout << "Refreshed cache for MockJob" << " with " << m_cache.size() << " components" << std::endl;
    }

    bool IsCompleted() const { return m_completed; }
    void Complete() { m_completed = true; }
    
private:
    bool m_completed;
};

// Mock system for testing
class MockSystem : public System<MockComponent> {
public:
    MockSystem(JobScheduler& scheduler) 
        : System<MockComponent>(scheduler), 
          m_jobsCreated(false), 
          m_jobsCompleted(false) {}
    
    void CreateJobs() override {
        m_jobsCreated = true;
        // Create some mock jobs
        // each job increments the value of the mock component by 1
        m_mockJobs.push_back(std::make_unique<MockJob>());
        
    }
    
private:
    bool m_jobsCreated;
    bool m_jobsCompleted;
    std::vector<std::unique_ptr<MockJob>> m_mockJobs;
};

// Test that the OnJobsCompleted signal is emitted when all jobs are completed
void TestOnJobsCompletedSignalEmitted() {
    //create 20 mock components
    for (int i = 0; i < 20; i++) {
        MockComponent::Create();
    }
    //create 20 mock components2
    for (int i = 0; i < 20; i++) {
        MockComponent2::Create();
    }
    
    JobScheduler scheduler;
    bool signalReceived = false;
    
    // Connect to the signal
    scheduler.OnJobsCompleted.connect([&signalReceived]() {
        signalReceived = true;
    });
    
    // Schedule a job
    auto job = std::make_unique<MockJob>();
    scheduler.ScheduleJob(std::move(static_cast<JobBase*>(job.release())));
    
    // Initially, the signal should not have been emitted
    assert(!signalReceived && "Signal should not be received before job completion");
    scheduler.Update(1.0f);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // Now the signal should have been emitted
    assert(signalReceived && "Signal should be received after job completion");
}

} // namespace JobSystem

// Main function to run all tests
int main() {
    try {
        JobSystem::TestOnJobsCompletedSignalEmitted();
        
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
} 