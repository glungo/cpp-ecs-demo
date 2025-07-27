#include "logger.h"
#include "LogStream.h"

namespace Logging {
    // Implement the Logger methods that return LogStream
    LogStream Logger::debug() { return LogStream(LogLevel::Debug, *this); }
    LogStream Logger::info() { return LogStream(LogLevel::Info, *this); }
    LogStream Logger::warning() { return LogStream(LogLevel::Warning, *this); }
    LogStream Logger::error() { return LogStream(LogLevel::Error, *this); }
    LogStream Logger::fatal() { return LogStream(LogLevel::Fatal, *this); }
} 