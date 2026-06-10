#include "media/player.hpp"

#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

Player::Player() = default;

Player::~Player() {
    if (formatContext_) {
        avformat_close_input(&formatContext_);
    }
}

bool Player::open(const std::string& path) {
    filePath_ = path;

    int result = avformat_open_input(&formatContext_, filePath_.c_str(), nullptr, nullptr);
    if (result < 0) {
        char errorBuffer[AV_ERROR_MAX_STRING_SIZE]{};
        av_strerror(result, errorBuffer, sizeof(errorBuffer));

        std::cerr << "FFmpeg failed to open file: " << filePath_ << "\n";
        std::cerr << "Error: " << errorBuffer << "\n";
        return false;
    }

    result = avformat_find_stream_info(formatContext_, nullptr);
    if (result < 0) {
        std::cerr << "FFmpeg failed to read stream info.\n";
        return false;
    }

    return true;
}

void Player::printInfo() const {
    if (!formatContext_) {
        std::cout << "No file is open.\n";
        return;
    }

    std::cout << "File opened with FFmpeg.\n";
    std::cout << "Path: " << filePath_ << "\n";

    if (formatContext_->duration != AV_NOPTS_VALUE) {
        double seconds = static_cast<double>(formatContext_->duration) / AV_TIME_BASE;
        std::cout << "Duration: " << seconds << " seconds\n";
    }

    for (unsigned int i = 0; i < formatContext_->nb_streams; ++i) {
        const AVStream* stream = formatContext_->streams[i];
        const AVCodecParameters* params = stream->codecpar;

        if (params->codec_type == AVMEDIA_TYPE_VIDEO) {
            std::cout << "Video stream #" << i << "\n";
            std::cout << "  Codec: " << avcodec_get_name(params->codec_id) << "\n";
            std::cout << "  Resolution: " << params->width << "x" << params->height << "\n";

            if (stream->avg_frame_rate.den != 0) {
                double fps = av_q2d(stream->avg_frame_rate);
                std::cout << "  FPS: " << fps << "\n";
            }
        }

        if (params->codec_type == AVMEDIA_TYPE_AUDIO) {
            std::cout << "Audio stream #" << i << "\n";
            std::cout << "  Codec: " << avcodec_get_name(params->codec_id) << "\n";
            std::cout << "  Sample rate: " << params->sample_rate << " Hz\n";
            std::cout << "  Channels: " << params->ch_layout.nb_channels << "\n";
        }
    }
}