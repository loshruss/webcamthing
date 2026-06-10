#include "app.hpp"

#include <iostream>
#include "media/player.hpp"

int App::run(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: webcamthing <video-file>\n";
        return 1;
    }

    const char* videoPath = argv[1];

    std::cout << "Opening file: " << videoPath << "\n";

    Player player;

    if (!player.open(videoPath)) {
        std::cerr << "Failed to open video file.\n";
        return 1;
    }

    player.printInfo();

    return 0;
}