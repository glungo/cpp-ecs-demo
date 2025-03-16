#include "logger.h"
#include "LogStream.h"

namespace Logging {
    // Implement the Logger methods that return LogStream
    inline LogStream Logger::debug() { return LogStream(LogLevel::Debug, *this); }
    inline LogStream Logger::info() { return LogStream(LogLevel::Info, *this); }
    inline LogStream Logger::warning() { return LogStream(LogLevel::Warning, *this); }
    inline LogStream Logger::error() { return LogStream(LogLevel::Error, *this); }
    inline LogStream Logger::fatal() { return LogStream(LogLevel::Fatal, *this); }
    
    // Implement the LogStream::flush method
    inline void LogStream::flush() {
        if (m_active && !m_buffer.str().empty()) {
            m_logger.log(m_level, m_buffer.str());
            m_buffer.str("");  // Clear the buffer
        }
    }
} 