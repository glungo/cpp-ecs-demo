#pragma once
#include <functional>
#include <string>
#include <vector>
#include <tuple>
#include <memory>

namespace JobSystem {

//JobCache is a vector of all the components that are needed for the job
template<typename... Components>
using JobCache = std::vector<std::tuple<Components*...>>;

// Base class for all jobs (non-templated)
class JobBase {
public:
    JobBase(const std::string& name) : m_name(name) {}
    virtual ~JobBase() = default;
    
    virtual void Execute(float dt) = 0;
    const std::string& GetName() const { return m_name; }
    
    // Virtual method to refresh the job's component cache
    virtual void RefreshCache() = 0;

protected:
    std::string m_name;
};

// Templated job implementation
template<typename... Components>
class Job : public JobBase {
public:
    Job(const std::string& name, 
        std::function<void(float, const std::vector<std::tuple<Components*...>>&)> func)
        : JobBase(name), m_function(func) {}

    void Execute(float dt) override {
        m_function(dt, m_cache);
    }

    void SetCache(const std::vector<std::tuple<Components*...>>& cache) {
        m_cache = cache;
    }
    
    // Implementation of cache refresh
    void RefreshCache() override {
        // This would be implemented to query the ECS for updated components
        // For example:
        // m_cache = ComponentManager::GetComponentsForJob<Components...>();
    }

protected:
    std::function<void(float, const std::vector<std::tuple<Components*...>>&)> m_function;
    std::vector<std::tuple<Components*...>> m_cache;
};

} // namespace JobSystem