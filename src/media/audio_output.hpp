#pragma once

#include <cstdint>
#include <cstddef>

using SDL_AudioDeviceID = unsigned int;

class AudioOutput {
public:
    AudioOutput();
    ~AudioOutput();

    AudioOutput(const AudioOutput&) = delete;
    AudioOutput& operator=(const AudioOutput&) = delete;

    bool open(int sampleRate, int channels);
    void queueAudio(const uint8_t* data, std::size_t size);
    int queuedBytes() const;

private:
    SDL_AudioDeviceID deviceId_ = 0;
};