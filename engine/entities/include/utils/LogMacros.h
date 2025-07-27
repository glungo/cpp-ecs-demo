#pragma once

#include "LogLevel.h"
#include "logger.h"
#include "LogStream.h"

namespace Logging {
    // Helper function to get a LogStream for a specific level
    inline LogStream GetLogStream(LogLevel level) {
        return LogStream(level, Logger::getInstance());
    }
}

//global namespace
// Global convenience macros without parentheses
#define LOG Logging::GetLogStream(Logging::LogLevel::Debug)
#define LOG_DEBUG Logging::GetLogStream(Logging::LogLevel::Debug)
#define LOG_INFO Logging::GetLogStream(Logging::LogLevel::Info)
#define LOG_WARNING Logging::GetLogStream(Logging::LogLevel::Warning)
#define LOG_ERROR Logging::GetLogStream(Logging::LogLevel::Error)
#define LOG_FATAL Logging::GetLogStream(Logging::LogLevel::Fatal)

// End marker
#define LOG_END Logging::LogEnd() 