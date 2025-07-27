#pragma once

#include <queue>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include "job.h"
#include "utils/signal.h"
#include <iostream>

namespace JobSystem {

// Simple signal/slot implementation with connection management
template<typename... Args>
class Signal {
public:
    using Callback = std::function<void(Args...)>;
    using ConnectionId = size_t;
    
    ConnectionId connect(Callback callback) {
        std::lock_guard<std::mutex> lock(m_mutex);
        ConnectionId id = m_nextId++;
        m_callbacks[id] = std::move(callback);
        return id;
    }
    
    template<typename T>
    ConnectionId connect(T* instance, void (T::*memberFunction)(Args...)) {
        return connect([instance, memberFunction](Args... args) {
            (instance->*memberFunction)(args...);
        });
    }
    
    void disconnect(ConnectionId id) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_callbacks.erase(id);
    }
    
    void disconnectAll() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_callbacks.clear();
    }
    
    void emit(Args... args) {
        // Make a copy of callbacks to avoid issues if callbacks modify the signal
        std::unordered_map<ConnectionId, Callback> callbacksCopy;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            callbacksCopy = m_callbacks;
        }
        
        for (auto& pair : callbacksCopy) {
            if (pair.second) {
                pair.second(args...);
            }
        }
    }
    
private:
    std::unordered_map<ConnectionId, Callback> m_callbacks;
    ConnectionId m_nextId = 0;
    std::mutex m_mutex;
};

class JobBase;
template<typename... Components> class Job;

class JobScheduler {
public:
    JobScheduler(size_t numThreads = std::thread::hardware_concurrency())
        : m_running(true) {
        // Start worker threads
        for (size_t i = 0; i < numThreads; ++i) {
            m_threads.emplace_back(&JobScheduler::WorkerThread, this);
        }
    }

    ~JobScheduler() {
        // Signal threads to stop and wait for them
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_running = false;
        }
        m_condition.notify_all();
        for (auto& thread : m_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        // Clear all signal connections to prevent callbacks to destroyed objects
        OnJobsCompleted.disconnectAll();

        // Clear all jobs
        {
            std::lock_guard<std::mutex> lock(m_jobsMutex);
            m_ownedJobs.clear();  // This will properly delete all owned jobs
            m_jobs.clear();       // Clear the raw pointers
        }
        //clear the job queue
        std::queue<std::unique_ptr<JobBase>> emptyQueue;
        std::swap(m_jobQueue, emptyQueue);
        //clear the completed jobs
        std::queue<std::unique_ptr<JobBase>> emptyCompletedQueue;
        std::swap(m_completedJobs, emptyCompletedQueue);
    }

    // Schedule a job for execution
    void ScheduleJob(JobBase* job) {
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_jobQueue.push(std::unique_ptr<JobBase>(job));
        }
        m_condition.notify_one();
    }

    // Run all scheduled jobs with the given delta time
    void Update(float dt) {
        m_deltaTime = dt;
        
        // Process any completed jobs
        std::lock_guard<std::mutex> lock(m_completedMutex);
        while (!m_completedJobs.empty()) {
            m_completedJobs.front()->PostExecute();
            m_completedJobs.pop();
        }
        OnJobsCompleted.emit();
    }

    // Signal that will be emitted when all jobs are completed
    Signal<> OnJobsCompleted;

    // Method to check if jobs are done and emit the signal
    void CheckJobsCompletion() {
        bool isEmpty = false;
        {
            std::lock_guard<std::mutex> lock(m_jobsMutex);
            isEmpty = m_jobs.empty();
        }
        
        if (isEmpty) {
            OnJobsCompleted.emit();
        }
    }
    
    // Method to notify that a specific job is completed
    void NotifyJobCompleted(JobBase* job) {
        // Remove the job from the active jobs list
        {
            std::lock_guard<std::mutex> lock(m_jobsMutex);
            
            // Remove from the raw pointer list
            for (auto it = m_jobs.begin(); it != m_jobs.end(); ++it) {
                if (*it == job) {
                    m_jobs.erase(it);
                    break;
                }
            }
            
            // Find and remove from the owned jobs list
            for (auto it = m_ownedJobs.begin(); it != m_ownedJobs.end(); ++it) {
                if (it->get() == job) {
                    m_ownedJobs.erase(it);
                    break;
                }
            }
        }
        
        // Check if all jobs are completed
        CheckJobsCompletion();
    }

private:
    void WorkerThread() {
        while (true) {
            std::unique_ptr<JobBase> job;
            
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_condition.wait(lock, [this] { 
                    return !m_running || !m_jobQueue.empty(); 
                });
                
                if (!m_running && m_jobQueue.empty()) {
                    return;
                }
                // wait for all dependencies to be met
                if (!m_jobQueue.front()->DependenciesMet())
                {
                    return;
                }
                job = std::move(m_jobQueue.front());
                // Refresh the job's cache before scheduling
                job->RefreshCache();
                m_jobQueue.pop();
            }
            
            // Execute the job
            job->Execute(m_deltaTime);
            
            // Store completed job
            {
                std::lock_guard<std::mutex> lock(m_completedMutex);
                m_completedJobs.push(std::move(job));
            }
        }
    }

    std::queue<std::unique_ptr<JobBase>> m_jobQueue;
    std::queue<std::unique_ptr<JobBase>> m_completedJobs;
    std::vector<std::thread> m_threads;
    std::mutex m_queueMutex;
    std::mutex m_completedMutex;
    std::condition_variable m_condition;
    bool m_running;
    float m_deltaTime = 0.0f;

    std::vector<JobBase*> m_jobs;                     // Raw pointers for quick access
    std::vector<std::shared_ptr<JobBase>> m_ownedJobs; // Shared pointers for ownership
    std::mutex m_jobsMutex;
};

} // namespace JobSystem