#pragma once

#include <cstdint>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class VideoRenderer {
public:
    VideoRenderer();
    ~VideoRenderer();

    VideoRenderer(const VideoRenderer&) = delete;
    VideoRenderer& operator=(const VideoRenderer&) = delete;

    bool open(int width, int height);
    bool renderFrame(const uint8_t* rgbaPixels, int pitch);
    bool shouldClose() const;

private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;

    bool shouldClose_ = false;
};