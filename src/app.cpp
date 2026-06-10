#include "app.hpp"

#include <iostream>
#include <string>
#include "media/player.hpp"
#include "media/audio_output.hpp"

int App::run(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage:\n";
        std::cout << "  webcamthing --list-audio-devices\n";
        std::cout << "  webcamthing <video-file> [--audio-device <index>]\n";
        return 1;
    }

    if (std::string(argv[1]) == "--list-audio-devices") {
        AudioOutput::listDevices();
        return 0;
    }

    const char* videoPath = argv[1];
    int audioDeviceIndex = -1;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--audio-device") {
            if (i + 1 >= argc) {
                std::cerr << "--audio-device requires an index.\n";
                return 1;
            }

            audioDeviceIndex = std::stoi(argv[++i]);
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            return 1;
        }
    }

    std::cout << "Opening file: " << videoPath << "\n";

    Player player;

    if (!player.open(videoPath)) {
        std::cerr << "Failed to open video file.\n";
        return 1;
    }

    player.printInfo();

    if (!player.previewVideoWithAudio(300, audioDeviceIndex)) {
        std::cerr << "Failed while previewing video with audio.\n";
        return 1;
    }

    return 0;
}