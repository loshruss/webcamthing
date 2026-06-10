#include "app.hpp"

#include <iostream>
#include <string>
#include "media/player.hpp"
#include "media/audio_output.hpp"

int App::run(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage:\n";
        std::cout << "  webcamthing --list-audio-devices\n";
        std::cout << "  webcamthing <video-file> [--audio-device <index>] [--frames <count>] [--loop]\n";
        return 1;
    }

    if (std::string(argv[1]) == "--list-audio-devices") {
        AudioOutput::listDevices();
        return 0;
    }

    const char* videoPath = argv[1];

    int audioDeviceIndex = -1;
    int frameCount = 300;
    bool loop = false;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--audio-device") {
            if (i + 1 >= argc) {
                std::cerr << "--audio-device requires an index.\n";
                return 1;
            }

            audioDeviceIndex = std::stoi(argv[++i]);
        } else if (arg == "--frames") {
            if (i + 1 >= argc) {
                std::cerr << "--frames requires a count.\n";
                return 1;
            }

            frameCount = std::stoi(argv[++i]);

            if (frameCount <= 0) {
                std::cerr << "--frames must be greater than 0.\n";
                return 1;
            }
        } else if (arg == "--loop") {
            loop = true;
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

    do {
        if (!player.previewVideoWithAudio(frameCount, audioDeviceIndex)) {
            std::cerr << "Failed while previewing video with audio.\n";
            return 1;
        }

        if (loop) {
            std::cout << "Looping...\n";

            if (!player.seekToStart()) {
                return 1;
            }
        }
    } while (loop);

    return 0;
}