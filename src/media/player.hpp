#pragma once

#include <string>

struct AVFormatContext;

class Player {
public:
    Player();
    ~Player();

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    bool open(const std::string& path);
    void printInfo() const;

private:
    std::string filePath_;
    AVFormatContext* formatContext_ = nullptr;
};