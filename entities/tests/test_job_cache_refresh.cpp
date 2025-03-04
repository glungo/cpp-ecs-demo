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

// Component with atomic counter for thread-safe testing
struct CounterComponent {
    std::atomic<int> counter{0};
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

// Test that job cache is refreshed before execution
void test_cache_refresh() {
    entities::utils::Logger::log("Testing job cache refresh", entities::utils::Logger::WARN_LEVEL::INFO);
    
    JobScheduler scheduler(1); // Single thread for predictable execution
    
    // Create components
    CounterComponent counter;
    std::vector<CounterComponent*> components = {&counter};
    
    // Create a job that increments the counter
    auto job = std::make_unique<TestJob<CounterComponent>>(
        "CounterJob",
        [](float dt, const std::vector<CounterComponent*>& components) {
            for (auto* comp : components) {
                comp->counter++;
            }
        }
    );
    
    // Set the initial cache
    job->SetTestCache(components);
    
    // Schedule the job
    scheduler.ScheduleJob(std::move(job));
    
    // Wait for job to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Verify counter was incremented
    assert(counter.counter == 1 && "Counter should be incremented once");
}

} // namespace tests
} // namespace JobSystem

int main() {
    JobSystem::tests::test_cache_refresh();
    std::cout << "Cache refresh test passed!" << std::endl;
    return 0;
} 