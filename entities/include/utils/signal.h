#pragma once

#include <functional>
#include <vector>

namespace Utils {
// Simple signal/slot implementation
template<typename... Args>
class Signal {
public:
    using Callback = std::function<void(Args...)>;
    
    template<typename T>
    void connect(T* instance, void (T::*memberFunction)(Args...)) {
        m_callbacks.push_back([instance, memberFunction](Args... args) {
            (instance->*memberFunction)(args...);
        });
    }
    
    void connect(Callback callback) {
        m_callbacks.push_back(std::move(callback));
    }
    
    void emit(Args... args) {
        for (auto& callback : m_callbacks) {
            callback(args...);
        }
    }
    
private:
    std::vector<Callback> m_callbacks;
};
} // namespace Utils