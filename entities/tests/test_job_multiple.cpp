#include <cassert>
#include <chrono>
#include <thread>
#include <atomic>
#include <iostream>
#include "include/job.h"
#include "include/job_scheduler.h"
#include "include/utils/LogMacros.h"
namespace JobSystem {
namespace tests {

// Simple component type for testing
struct TestComponent {
    int value = 0;
};

// Test job that implements RefreshCache
template<typename T>
class TestJob : public JobBase {
public:
    TestJob(const std::string& name, std::function<void(float, const std::vector<T*>&)> func)
        : JobBase(name), m_function(func) {}

    void Execute(float dt) override {
        m_function(dt, m_cache);
    }

    void RefreshCache() override {
        // In a real system, this would query the component manager
        // For testing, we'll just use our manually set cache
    }

    void SetTestCache(const std::vector<T*>& cache) {
        m_cache = cache;
    }

private:
    std::function<void(float, const std::vector<T*>&)> m_function;
    std::vector<T*> m_cache;
};

// Test that multiple jobs can be scheduled and executed
void test_multiple_jobs() {
    LOG << "Testing multiple jobs" << LOG_END;
    
    JobScheduler scheduler(2);
    std::atomic<int> sum{0};
    
    // Create 5 jobs that add different values to the sum
    for (int i = 1; i <= 5; i++) {
        auto job = std::make_unique<TestJob<TestComponent>>(
            "AddJob" + std::to_string(i),
            [i, &sum](float dt, const std::vector<TestComponent*>& components) {
                sum += i;
            }
        );
        
        scheduler.ScheduleJob(std::move(static_cast<JobBase*>(job.release())));
    }
    
    // Wait for all jobs to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Verify sum is correct (1+2+3+4+5 = 15)
    assert(sum == 15 && "Sum should be 15");
}

} // namespace tests
} // namespace JobSystem

int main() {
    JobSystem::tests::test_multiple_jobs();
    LOG << "Multiple jobs test passed!" << LOG_END;
    return 0;
} 