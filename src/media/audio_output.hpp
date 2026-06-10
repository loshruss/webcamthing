#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

using SDL_AudioDeviceID = unsigned int;

class AudioOutput {
public:
    AudioOutput();
    ~AudioOutput();

    AudioOutput(const AudioOutput&) = delete;
    AudioOutput& operator=(const AudioOutput&) = delete;

    static void listDevices();

    bool open(int sampleRate, int channels, int deviceIndex = -1);
    void queueAudio(const uint8_t* data, std::size_t size);
    int queuedBytes() const;
    int bytesPerSecond() const;

private:
    SDL_AudioDeviceID deviceId_ = 0;

    int sampleRate_ = 0;
    int channels_ = 0;
    int bytesPerSample_ = 2;
};