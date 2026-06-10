#pragma once

#include <string>

struct AVFormatContext;
struct AVCodecContext;

class Player {
public:
    Player();
    ~Player();

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    bool open(const std::string& path);
    void printInfo() const;

    bool decodeSomeVideoFrames(int maxFrames);
    bool previewVideo(int maxFrames);
    bool previewVideoWithAudio(int maxVideoFrames, int audioDeviceIndex = -1);

private:
    int videoFrameDelayMs() const;

    std::string filePath_;

    AVFormatContext* formatContext_ = nullptr;

    AVCodecContext* videoCodecContext_ = nullptr;
    AVCodecContext* audioCodecContext_ = nullptr;

    int videoStreamIndex_ = -1;
    int audioStreamIndex_ = -1;
};