#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include "logger.h" // For LogLevel enum

namespace Logging {

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