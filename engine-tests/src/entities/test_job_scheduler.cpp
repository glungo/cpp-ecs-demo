#include "include/job_scheduler.h"
#include "include/component.h"
#include <cassert>
#include <iostream>
#include <memory>
using namespace entities;

namespace JobSystem {
namespace tests {


DEFINE_COMPONENT(MockComponent, 100)
    COMPONENT_MEMBER(int, m_value) = 0;
END_COMPONENT

DEFINE_COMPONENT(MockComponent2, 100)
    COMPONENT_MEMBER(int, m_value2) = 0;
END_COMPONENT

// Mock job class for testing
class MockJob : public Job<MockComponent> {
public:
    MockJob() : Job<MockComponent>("MockJob", 
        [](float dt, const std::vector<std::tuple<MockComponent*>>& components) {
            for (auto& component : components) {
                std::get<0>(component)->m_value += 1;
            }
        }
    ) {}
    
    virtual void RefreshCache() override {
        auto components = MockComponent::GetAll();
        m_cache = std::vector<std::tuple<MockComponent*>>(components.begin(), components.end());
    }

    bool IsCompleted() const { return m_completed; }
    void Complete() { m_completed = true; }
    
private:
    bool m_completed;
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
    for(int i = 0; i < 20; i++) {
        auto job = std::make_unique<MockJob>();
        scheduler.ScheduleJob(std::move(static_cast<JobBase*>(job.release())));
    }
    
    // Initially, the signal should not have been emitted
    assert(!signalReceived && "Signal should not be received before job completion");
    scheduler.Update(1.0f);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // Now the signal should have been emitted
    assert(signalReceived && "Signal should be received after job completion");
    for(auto& component : MockComponent::GetAll()) {
        assert(component->m_value == 20 && "Component value should be 20");
    }
    for(auto& component : MockComponent2::GetAll()) {
        assert(component->m_value2 == 0 && "Component value should be 0");
    }
}

} // namespace tests
} // namespace Jobsystem

// Main function to run all tests
int main() {
    try {
        JobSystem::tests::TestOnJobsCompletedSignalEmitted();
        
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
} 
