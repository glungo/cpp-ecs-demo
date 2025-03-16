#pragma once

#include "LogLevel.h"
#include <string>
#include <mutex>
#include <vector>
#include <memory>
#include "LogSink.h"
#include "singleton.h"
class LogStream;

namespace Logging {
class Logger: public Singleton<Logger> {
    DECLARE_SINGLETON(Logger)
public:
    
    // Set the minimum log level to display
    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_minLevel = level;
    }
    
    // Add a log sink (console, file, etc.)
    void addSink(std::shared_ptr<LogSink> sink) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sinks.push_back(sink);
    }
    
    // Log a message with a specific level
    void log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (level < m_minLevel) {
            return;  // Skip messages below the minimum level
        }
        
        for (auto& sink : m_sinks) {
            sink->write(level, message);
        }
    }

private:
    // Private constructor that will be called by getInstance()
    Logger() : m_minLevel(LogLevel::Info) {
        // Add console sink by default
        addSink(std::make_shared<ConsoleSink>());
    }
    LogLevel m_minLevel;
    std::mutex m_mutex;
    std::vector<std::shared_ptr<LogSink>> m_sinks;
};

} // namespace Logging
