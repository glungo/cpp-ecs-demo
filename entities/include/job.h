#pragma once
#include <functional>
#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <iostream>
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

    virtual void PostExecute()
    {
        for (auto& callback : m_onJobCompletedCallbacks) 
        {
            callback();
        }
        m_onJobCompletedCallbacks.clear();
    }
    const std::string& GetName() const { return m_name; }
    
    // Virtual method to refresh the job's component cache
    virtual void RefreshCache() = 0;

     void AddDependency(JobBase* dependency)
    {
        m_dependencies.push_back(dependency);
    }

    bool DependenciesMet() const 
    {
        for (auto& dependency : m_dependencies) {
            if (!dependency->IsCompleted()) {
                return false;
            }
        }
        return true;
    }

    void SetCompleted()
    {
        m_completed = true;
    }

    bool IsCompleted() const 
    {
        return m_completed;
    }

    void AddOnJobCompletedCallback(std::function<void()> callback) {
        m_onJobCompletedCallbacks.push_back(callback);
    }

protected:
    std::string m_name;
    std::vector<std::function<void()>> m_onJobCompletedCallbacks;
    std::vector<JobBase*> m_dependencies;
    bool m_completed = false;
};

// Templated job implementation
template<typename... Components>
class Job : public JobBase {
public:
    Job(const std::string& name, 
        std::function<void(float, const std::vector<std::tuple<Components*...>>&)> func)
        : JobBase(name), m_function(func) {}

    void Execute(float dt) override 
    {
        m_function(dt, m_cache);
    }

    void SetCache(const std::vector<std::tuple<Components*...>>& cache) {
        m_cache = cache;
    }

    void RefreshCache() override {
        //need component manager to do this
    }
protected:
    std::function<void(float, const std::vector<std::tuple<Components*...>>&)> m_function;
    std::vector<std::tuple<Components*...>> m_cache;
};

} // namespace JobSystem