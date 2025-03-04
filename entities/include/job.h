#pragma once
#include <functional>
#include <string>
#include <vector>
#include <tuple>
namespace JobSystem {

//JobCache is a vector of all the components that are needed for the job
template<typename... Components>
using JobCache = std::vector<std::tuple<Components*...>>;

//Job is a class that contains a function that will be executed by the job system
template<typename... Components>
class Job {
public:
    Job(const std::string& name, 
        std::function<void(float, const std::vector<std::tuple<Components*...>>&)> func)
        : m_name(name), m_function(func) {}

    void Execute(float dt) {
        m_function(dt, m_cache);
    }

    const std::string& GetName() const { return m_name; }

    void SetCache(const std::vector<std::tuple<Components*...>>& cache) {
        m_cache = cache;
    }

    virtual ~Job() = default;

protected:
    std::string m_name;
    std::function<void(float, const std::vector<std::tuple<Components*...>>&)> m_function;
    std::vector<std::tuple<Components*...>> m_cache;
};

} // namespace JobSystem