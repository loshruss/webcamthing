#include "media/player.hpp"
#include "media/video_renderer.hpp"

#include <iostream>
#include <vector>
#include <SDL2/SDL.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

Player::Player() = default;

Player::~Player() {
    if (videoCodecContext_) {
        avcodec_free_context(&videoCodecContext_);
    }

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

    result = av_find_best_stream(
        formatContext_,
        AVMEDIA_TYPE_VIDEO,
        -1,
        -1,
        nullptr,
        0
    );

    if (result < 0) {
        std::cerr << "No video stream found.\n";
        return false;
    }

    videoStreamIndex_ = result;

    AVStream* videoStream = formatContext_->streams[videoStreamIndex_];
    const AVCodecParameters* codecParams = videoStream->codecpar;

    const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
    if (!codec) {
        std::cerr << "Could not find decoder for video codec.\n";
        return false;
    }

    videoCodecContext_ = avcodec_alloc_context3(codec);
    if (!videoCodecContext_) {
        std::cerr << "Could not allocate video codec context.\n";
        return false;
    }

    result = avcodec_parameters_to_context(videoCodecContext_, codecParams);
    if (result < 0) {
        std::cerr << "Could not copy codec parameters to codec context.\n";
        return false;
    }

    result = avcodec_open2(videoCodecContext_, codec, nullptr);
    if (result < 0) {
        char errorBuffer[AV_ERROR_MAX_STRING_SIZE]{};
        av_strerror(result, errorBuffer, sizeof(errorBuffer));

        std::cerr << "Could not open video decoder.\n";
        std::cerr << "Error: " << errorBuffer << "\n";
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

bool Player::decodeSomeVideoFrames(int maxFrames) {
    if (!formatContext_ || !videoCodecContext_ || videoStreamIndex_ < 0) {
        std::cerr << "Player is not ready to decode video.\n";
        return false;
    }

    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    if (!packet || !frame) {
        std::cerr << "Could not allocate FFmpeg packet/frame.\n";

        if (packet) {
            av_packet_free(&packet);
        }

        if (frame) {
            av_frame_free(&frame);
        }

        return false;
    }

    int decodedFrames = 0;

    while (av_read_frame(formatContext_, packet) >= 0 && decodedFrames < maxFrames) {
        if (packet->stream_index == videoStreamIndex_) {
            int result = avcodec_send_packet(videoCodecContext_, packet);

            if (result < 0) {
                char errorBuffer[AV_ERROR_MAX_STRING_SIZE]{};
                av_strerror(result, errorBuffer, sizeof(errorBuffer));

                std::cerr << "Failed to send packet to decoder.\n";
                std::cerr << "Error: " << errorBuffer << "\n";

                av_packet_unref(packet);
                av_frame_free(&frame);
                av_packet_free(&packet);
                return false;
            }

            while (result >= 0) {
                result = avcodec_receive_frame(videoCodecContext_, frame);

                if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
                    break;
                }

                if (result < 0) {
                    char errorBuffer[AV_ERROR_MAX_STRING_SIZE]{};
                    av_strerror(result, errorBuffer, sizeof(errorBuffer));

                    std::cerr << "Failed to receive frame from decoder.\n";
                    std::cerr << "Error: " << errorBuffer << "\n";

                    av_packet_unref(packet);
                    av_frame_free(&frame);
                    av_packet_free(&packet);
                    return false;
                }

                ++decodedFrames;

                std::cout << "Decoded video frame " << decodedFrames
                          << " | "
                          << frame->width << "x" << frame->height
                          << " | format " << frame->format
                          << "\n";

                av_frame_unref(frame);

                if (decodedFrames >= maxFrames) {
                    break;
                }
            }
        }

        av_packet_unref(packet);
    }

    std::cout << "Decoded " << decodedFrames << " video frames successfully.\n";

    av_frame_free(&frame);
    av_packet_free(&packet);

    return decodedFrames > 0;
}

bool Player::previewVideo(int maxFrames) {
    if (!formatContext_ || !videoCodecContext_ || videoStreamIndex_ < 0) {
        std::cerr << "Player is not ready to preview video.\n";
        return false;
    }

    VideoRenderer renderer;

    if (!renderer.open(videoCodecContext_->width, videoCodecContext_->height)) {
        return false;
    }

    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    if (!packet || !frame) {
        std::cerr << "Could not allocate FFmpeg packet/frame.\n";
        if (packet) av_packet_free(&packet);
        if (frame) av_frame_free(&frame);
        return false;
    }

    SwsContext* swsContext = sws_getContext(
        videoCodecContext_->width,
        videoCodecContext_->height,
        videoCodecContext_->pix_fmt,
        videoCodecContext_->width,
        videoCodecContext_->height,
        AV_PIX_FMT_RGBA,
        SWS_BILINEAR,
        nullptr,
        nullptr,
        nullptr
    );

    if (!swsContext) {
        std::cerr << "Could not create swscale context.\n";
        av_frame_free(&frame);
        av_packet_free(&packet);
        return false;
    }

    const int width = videoCodecContext_->width;
    const int height = videoCodecContext_->height;
    const int rgbaPitch = width * 4;

    std::vector<uint8_t> rgbaBuffer(rgbaPitch * height);

    uint8_t* destData[4] = {
        rgbaBuffer.data(),
        nullptr,
        nullptr,
        nullptr
    };

    int destLinesize[4] = {
        rgbaPitch,
        0,
        0,
        0
    };

    int decodedFrames = 0;

    while (av_read_frame(formatContext_, packet) >= 0 && decodedFrames < maxFrames) {
        if (packet->stream_index == videoStreamIndex_) {
            int result = avcodec_send_packet(videoCodecContext_, packet);

            if (result < 0) {
                std::cerr << "Failed to send packet to decoder.\n";
                av_packet_unref(packet);
                break;
            }

            while (result >= 0) {
                result = avcodec_receive_frame(videoCodecContext_, frame);

                if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
                    break;
                }

                if (result < 0) {
                    std::cerr << "Failed to receive frame from decoder.\n";
                    av_packet_unref(packet);
                    sws_freeContext(swsContext);
                    av_frame_free(&frame);
                    av_packet_free(&packet);
                    return false;
                }

                sws_scale(
                    swsContext,
                    frame->data,
                    frame->linesize,
                    0,
                    height,
                    destData,
                    destLinesize
                );

                if (!renderer.renderFrame(rgbaBuffer.data(), rgbaPitch)) {
                    break;
                }

                ++decodedFrames;

                SDL_Delay(33);

                av_frame_unref(frame);

                if (renderer.shouldClose() || decodedFrames >= maxFrames) {
                    break;
                }
            }
        }

        av_packet_unref(packet);

        if (renderer.shouldClose()) {
            break;
        }
    }

    std::cout << "Previewed " << decodedFrames << " video frames.\n";

    sws_freeContext(swsContext);
    av_frame_free(&frame);
    av_packet_free(&packet);

    return decodedFrames > 0;
}