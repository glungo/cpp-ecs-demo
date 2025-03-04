#pragma once

#include <queue>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "job.h"

namespace JobSystem {

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
    }

    // Schedule a job for execution
    void ScheduleJob(std::unique_ptr<JobBase> job) {
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_jobQueue.push(std::move(job));
        }
        m_condition.notify_one();
    }

    // Run all scheduled jobs with the given delta time
    void Update(float dt) {
        m_deltaTime = dt;
        
        // Process any completed jobs
        std::lock_guard<std::mutex> lock(m_completedMutex);
        while (!m_completedJobs.empty()) {
            m_completedJobs.pop();
        }
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
};

} // namespace JobSystem