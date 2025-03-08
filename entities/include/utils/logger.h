#pragma once
#include <iostream>
namespace entities {
namespace utils {
    class Logger {
        public:
            enum class WARN_LEVEL {
                INFO,
                WARN,
                ERROR,
                FATAL
        };

        static void log(const std::string& message, WARN_LEVEL warn_level = WARN_LEVEL::INFO) {
            switch (warn_level) {
                case WARN_LEVEL::INFO:
                    std::cout << "[Entities][INFO] " << message << std::endl;
                    std::cout.flush();
                    break;
                case WARN_LEVEL::WARN:
                    std::cerr << "[Entities][WARN] " << message << std::endl;
                    std::cerr.flush();
                    break;
                case WARN_LEVEL::ERROR:
                    std::cerr << "[Entities][ERROR] " << message << std::endl;
                    std::cerr.flush();
                    break;
                case WARN_LEVEL::FATAL:
                    std::cerr << "[Entities][FATAL] " << message << std::endl;
                    std::cerr.flush();
                    break;
            }
        }
        private:
            Logger() = delete;  
            Logger(const Logger&) = delete;
            Logger& operator=(const Logger&) = delete;
    };
} // namespace utils
} // namespace entities
