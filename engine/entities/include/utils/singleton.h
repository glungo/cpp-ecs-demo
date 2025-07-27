#pragma once

#include <memory>

// Base singleton template class
template<typename T>
class Singleton {
protected:
    Singleton() = default;
    ~Singleton() = default;

    // Delete copy and move operations
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;
};

// Macro to declare a singleton class
#define DECLARE_SINGLETON(ClassName) \
public: \
    static ClassName& getInstance() { \
        static ClassName instance; \
        return instance; \
    } \
private: \
    friend class Singleton<ClassName>; \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete; \
    ClassName(ClassName&&) = delete; \
    ClassName& operator=(ClassName&&) = delete;

// Macro for a singleton class with a protected constructor
#define DECLARE_SINGLETON_WITH_PROTECTED_CONSTRUCTOR(ClassName) \
public: \
    static ClassName& getInstance() { \
        static ClassName instance; \
        return instance; \
    } \
protected: \
    friend class Singleton<ClassName>; \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete; \
    ClassName(ClassName&&) = delete; \
    ClassName& operator=(ClassName&&) = delete;