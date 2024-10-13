#pragma once

#include <fstream>
#include <mutex>

class FileWriter {
public:
    explicit FileWriter(const std::string &path) : file(path) {
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file");
        }
    }

    FileWriter& operator<<(const std::string &message) {
        std::lock_guard<std::mutex> lock(mutex);
        file << message << std::endl;
        return *this;
    }

private:
    std::mutex mutex;
    std::ofstream file;
};