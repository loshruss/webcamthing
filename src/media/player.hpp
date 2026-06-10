#pragma once

#include <string>

class Player {
public:
    bool open(const std::string& path);
    void printInfo() const;

private:
    std::string filePath_;
    bool isOpen_ = false;
};