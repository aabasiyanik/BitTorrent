#pragma once

#include <string>
#include <iostream>
#include <sstream>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static void log(LogLevel level, const std::string& message) {
        std::string level_str;
        switch (level) {
            case LogLevel::DEBUG:   level_str = "DEBUG"; break;
            case LogLevel::INFO:    level_str = "INFO"; break;
            case LogLevel::WARNING: level_str = "WARNING"; break;
            case LogLevel::ERROR:   level_str = "ERROR"; break;
        }
        
        std::cout << "[" << level_str << "] " << message << std::endl;
    }

    static void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    static void info(const std::string& message) { log(LogLevel::INFO, message); }
    static void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    static void error(const std::string& message) { log(LogLevel::ERROR, message); }
}; 