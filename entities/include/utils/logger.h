#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <mutex>
#include <vector>
#include <memory>
#include <fstream>

namespace Logging {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

// Forward declaration
class Logger;

class LogStream {
public:
    LogStream(LogLevel level, Logger& logger) 
        : m_level(level), m_logger(logger), m_active(true) {}
    
    // Move constructor
    LogStream(LogStream&& other) noexcept
        : m_level(other.m_level), 
          m_logger(other.m_logger), 
          m_buffer(std::move(other.m_buffer)),
          m_active(other.m_active) {
        other.m_active = false;  // Prevent the moved-from object from flushing
    }
    
    ~LogStream() {
        // Only flush if not explicitly ended and still active
        if (m_active) {
            flush();
        }
    }
    
    // Stream operator for any type that can be streamed
    template<typename T>
    LogStream& operator<<(const T& value) {
        if (m_active) {
            m_buffer << value;
        }
        return *this;
    }
    
    // Special handling for std::endl and other manipulators
    LogStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
        if (m_active) {
            manip(m_buffer);
        }
        return *this;
    }
    
    // End the log message and flush it
    void end() {
        if (m_active) {
            flush();
            m_active = false;
        }
    }
    
    // Flush the stream
    void flush() {
        if (m_active && !m_buffer.str().empty()) {
            m_logger.log(m_level, m_buffer.str());
            m_buffer.str("");  // Clear the buffer
        }
    }
    
private:
    LogLevel m_level;
    Logger& m_logger;
    std::stringstream m_buffer;
    bool m_active;  // Flag to prevent double-flushing
};

// End marker for log messages
struct LogEnd {
    void operator()(LogStream& stream) const {
        stream.end();
    }
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    // Get a stream for the specified log level
    LogStream debug() { return LogStream(LogLevel::Debug, *this); }
    LogStream info() { return LogStream(LogLevel::Info, *this); }
    LogStream warning() { return LogStream(LogLevel::Warning, *this); }
    LogStream error() { return LogStream(LogLevel::Error, *this); }
    LogStream fatal() { return LogStream(LogLevel::Fatal, *this); }
    
    // Set the minimum log level to display
    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_minLevel = level;
    }
    
    // Add a log sink (console, file, etc.)
    void addSink(std::shared_ptr<class LogSink> sink) {
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
    Logger() : m_minLevel(LogLevel::Info) {
        // Add console sink by default
        addSink(std::make_shared<ConsoleSink>());
    }
    
    ~Logger() = default;
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    LogLevel m_minLevel;
    std::mutex m_mutex;
    std::vector<std::shared_ptr<class LogSink>> m_sinks;
};

// Base class for log sinks
class LogSink {
public:
    virtual ~LogSink() = default;
    virtual void write(LogLevel level, const std::string& message) = 0;
    
protected:
    std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::Debug:   return "DEBUG";
            case LogLevel::Info:    return "INFO";
            case LogLevel::Warning: return "WARNING";
            case LogLevel::Error:   return "ERROR";
            case LogLevel::Fatal:   return "FATAL";
            default:                return "UNKNOWN";
        }
    }
};

// Console sink implementation
class ConsoleSink : public LogSink {
public:
    void write(LogLevel level, const std::string& message) override {
        std::ostream& out = (level >= LogLevel::Error) ? std::cerr : std::cout;
        out << "[" << levelToString(level) << "] " << message << std::endl;
    }
};

// File sink implementation
class FileSink : public LogSink {
public:
    FileSink(const std::string& filename) : m_file(filename) {
        if (!m_file.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }
    
    ~FileSink() {
        if (m_file.is_open()) {
            m_file.close();
        }
    }
    
    void write(LogLevel level, const std::string& message) override {
        if (m_file.is_open()) {
            m_file << "[" << levelToString(level) << "] " << message << std::endl;
        }
    }
    
private:
    std::ofstream m_file;
};

} // namespace Logging

// Global convenience functions
inline Logging::LogStream LOG() {
    return Logging::Logger::getInstance().debug();
}

inline Logging::LogStream LOG_INFO() {
    return Logging::Logger::getInstance().info();
}

inline Logging::LogStream LOG_WARNING() {
    return Logging::Logger::getInstance().warning();
}

inline Logging::LogStream LOG_ERROR() {
    return Logging::Logger::getInstance().error();
}

inline Logging::LogStream LOG_FATAL() {
    return Logging::Logger::getInstance().fatal();
}

// Global end marker
inline Logging::LogEnd LOG_END;

// Stream operator for the end marker
inline Logging::LogStream& operator<<(Logging::LogStream& stream, const Logging::LogEnd&) {
    stream.end();
    return stream;
}
