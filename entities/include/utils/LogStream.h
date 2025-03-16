#pragma once

#include <sstream>
#include <string>
#include "include/utils/logger.h"

namespace Logging {

class LogStream
{
public:
    LogStream(LogLevel level, Logger &logger)
        : m_level(level), m_logger(logger), m_active(true) {}

    // Move constructor
    LogStream(LogStream &&other) noexcept
        : m_level(other.m_level),
            m_logger(other.m_logger),
            m_buffer(std::move(other.m_buffer)),
            m_active(other.m_active)
    {
        other.m_active = false; // Prevent the moved-from object from flushing
    }

    ~LogStream()
    {
        // Only flush if not explicitly ended and still active
        if (m_active)
        {
            flush();
        }
    }

    // Stream operator for any type that can be streamed
    template <typename T>
    LogStream &operator<<(const T &value)
    {
        if (m_active)
        {
            m_buffer << value;
        }
        return *this;
    }

    // Special handling for std::endl and other manipulators
    LogStream &operator<<(std::ostream &(*manip)(std::ostream &))
    {
        if (m_active)
        {
            manip(m_buffer);
        }
        return *this;
    }

    // End the log message and flush it
    void end()
    {
        if (m_active)
        {
            flush();
            m_active = false;
        }
    }

    // Flush the stream
    void flush()
    {
        if (m_active && !m_buffer.str().empty())
        {
            m_logger.log(m_level, m_buffer.str());
            m_buffer.str(""); // Clear the buffer
        }
    }

private:
    LogLevel m_level;
    Logger &m_logger;
    std::stringstream m_buffer;
    bool m_active; // Flag to prevent double-flushing
};

// End marker for log messages
struct LogEnd
{
    void operator()(LogStream &stream) const
    {
        stream.end();
    }
};

// Stream operator for the end marker
inline LogStream &operator<<(LogStream &stream, const LogEnd &)
{
    stream.end();
    return stream;
}



} // namespace Logging 