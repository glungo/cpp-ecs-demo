#include <cassert>
#include <chrono>
#include <thread>
#include <atomic>
#include <iostream>
#include "include/job.h"
#include "include/job_scheduler.h"
#include "include/utils/logger.h"

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

// Test that jobs execute in parallel
void test_parallel_execution() {
    entities::utils::Logger::log("Testing parallel job execution", entities::utils::Logger::WARN_LEVEL::INFO);
    
    JobScheduler scheduler(4); // Use 4 threads
    std::atomic<int> completedJobs{0};
    
    // Create 10 jobs that each sleep for 100ms
    // If executed sequentially, this would take 1000ms
    // If executed in parallel with 4 threads, should take ~300ms
    for (int i = 0; i < 10; i++) {
        auto job = std::make_unique<TestJob<TestComponent>>(
            "SleepJob" + std::to_string(i),
            [&completedJobs](float dt, const std::vector<TestComponent*>& components) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                completedJobs++;
            }
        );
        
        scheduler.ScheduleJob(std::move(static_cast<JobBase*>(job.release())));
    }
    
    // Measure execution time
    auto start = std::chrono::high_resolution_clock::now();
    
    // Wait for all jobs to complete (with timeout)
    int maxWaitMs = 1000; // 1 second max wait
    int waitedMs = 0;
    while (completedJobs < 10 && waitedMs < maxWaitMs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        waitedMs += 10;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    entities::utils::Logger::log("All jobs completed in " + std::to_string(duration) + "ms", 
                                entities::utils::Logger::WARN_LEVEL::INFO);
    
    // Verify all jobs completed
    assert(completedJobs == 10 && "All jobs should complete");
    
    // Verify execution was parallel (should take less than 500ms with 4 threads)
    assert(duration < 500 && "Jobs should execute in parallel");
}

} // namespace tests
} // namespace JobSystem

int main() {
    JobSystem::tests::test_parallel_execution();
    std::cout << "Parallel execution test passed!" << std::endl;
    return 0;
} 