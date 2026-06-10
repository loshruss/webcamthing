#include "media/player.hpp"

#include <filesystem>
#include <iostream>

bool Player::open(const std::string& path) {
    filePath_ = path;

    if (!std::filesystem::exists(filePath_)) {
        std::cerr << "File does not exist: " << filePath_ << "\n";
        isOpen_ = false;
        return false;
    }

    isOpen_ = true;
    return true;
}

void Player::printInfo() const {
    if (!isOpen_) {
        std::cout << "No file is open.\n";
        return;
    }

    std::cout << "File opened successfully.\n";
    std::cout << "Path: " << filePath_ << "\n";
    std::cout << "Milestone 1 status: basic file loading works.\n";
}