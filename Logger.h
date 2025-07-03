#pragma once
// Logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

class Logger {
private:
    std::ofstream file;
    static Logger* instance;
    static std::mutex mutex;

    Logger() {
        file.open("log.txt", std::ios::app);
    }

public:
    static Logger& getInstance() {
        std::lock_guard<std::mutex> lock(mutex);
        if (!instance)
            instance = new Logger();
        return *instance;
    }

    void log(const std::string& message) {
        file << "[LOG] " << message << std::endl;
    }

    ~Logger() {
        file.close();
    }
};

#endif
