#include "media/audio_output.hpp"

#include <iostream>
#include <SDL2/SDL.h>

AudioOutput::AudioOutput() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL audio init failed: " << SDL_GetError() << "\n";
    }
}

AudioOutput::~AudioOutput() {
    if (deviceId_ != 0) {
        SDL_CloseAudioDevice(deviceId_);
    }

    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

bool AudioOutput::open(int sampleRate, int channels) {
    SDL_AudioSpec desired{};
    desired.freq = sampleRate;
    desired.format = AUDIO_S16SYS;
    desired.channels = static_cast<Uint8>(channels);
    desired.samples = 4096;
    desired.callback = nullptr;

    SDL_AudioSpec obtained{};

    deviceId_ = SDL_OpenAudioDevice(
        nullptr,
        0,
        &desired,
        &obtained,
        0
    );

    if (deviceId_ == 0) {
        std::cerr << "SDL_OpenAudioDevice failed: " << SDL_GetError() << "\n";
        return false;
    }

    std::cout << "Opened audio output:\n";
    std::cout << "  Sample rate: " << obtained.freq << " Hz\n";
    std::cout << "  Channels: " << static_cast<int>(obtained.channels) << "\n";

    SDL_PauseAudioDevice(deviceId_, 0);

    return true;
}

void AudioOutput::queueAudio(const uint8_t* data, std::size_t size) {
    if (deviceId_ == 0 || !data || size == 0) {
        return;
    }

    SDL_QueueAudio(deviceId_, data, static_cast<Uint32>(size));
}

int AudioOutput::queuedBytes() const {
    if (deviceId_ == 0) {
        return 0;
    }

    return static_cast<int>(SDL_GetQueuedAudioSize(deviceId_));
}