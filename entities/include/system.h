namespace JobSystem {
    template<typename... Components>
    class System {
        public:
            System(JobScheduler& scheduler) : 
                m_scheduler(scheduler),
                m_completionConnectionId(0),
                m_isRunning(false) {}
            
            virtual ~System() {
                // Disconnect from the scheduler's signal when destroyed
                if (m_completionConnectionId != 0) {
                    m_scheduler.OnJobsCompleted.disconnect(m_completionConnectionId);
                }
            }
            
            virtual void CreateJobs() = 0;
            virtual void OnJobsCompleted() = 0;
            
            bool CanBeRun()
            {
                if (m_isRunning) return false;
                for (auto& dependency : m_dependencies) {
                    if (dependency->m_isRunning) return false;
                }
                return true;    
            }

            void Run()
            {
                if (!CanBeRun()) return;
                m_isRunning = true;
                CreateJobs();
                
                // Disconnect any previous connection to avoid multiple callbacks
                if (m_completionConnectionId != 0) {
                    m_scheduler.OnJobsCompleted.disconnect(m_completionConnectionId);
                }
                
                // Connect to the scheduler's OnJobsCompleted signal
                m_completionConnectionId = m_scheduler.OnJobsCompleted.connect(
                    [this]() {
                        this->OnJobsCompleted();
                        m_isRunning = false;
                    }
                );
                
                for (auto& job : m_jobs) {
                    m_scheduler.ScheduleJob(std::move(job));
                }
                
                // Check if we need to trigger completion immediately (no jobs)
                m_scheduler.CheckJobsCompletion();
            }

            void OnJobCompleted(JobBase* job)
            {
                // Notify the scheduler that this job is done
                m_scheduler.NotifyJobCompleted(job);
            }
            
        protected:
            JobScheduler& m_scheduler;
            bool m_canRunInParallel = false;
            std::vector<System*> m_dependencies;
            bool m_isRunning;
            std::vector<Job<Components...>*> m_jobs;
            
        private:
            // Store the connection ID so we can disconnect later
            typename Signal<>::ConnectionId m_completionConnectionId;
    };
} // namespace JobSystem