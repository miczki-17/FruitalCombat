#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <iostream>

// for ffmpeg includes

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

namespace game::utils
{
    class VideoPlayer
    {
    private:
        AVFormatContext* formatCtx = nullptr;
        AVCodecContext* codecCtx = nullptr;
        AVFrame* frame = nullptr;
        AVFrame* frameRGBA = nullptr;
        AVPacket* packet = nullptr;
        SwsContext* swsCtx = nullptr;

        int videoStreamIndex = -1;
        std::vector<uint8_t> pixelBuffer;

        
        sf::Texture texture;
        sf::Sprite sprite;
        bool isFinished = false;

        void cleanup()
        {
            if (swsCtx) { sws_freeContext(swsCtx); swsCtx = nullptr; }
            if (packet) { av_packet_free(&packet); packet = nullptr; }
            if (frameRGBA) { av_frame_free(&frameRGBA); frameRGBA = nullptr; }
            if (frame) { av_frame_free(&frame); frame = nullptr; }
            if (codecCtx) { avcodec_free_context(&codecCtx); codecCtx = nullptr; }
            if (formatCtx) { avformat_close_input(&formatCtx); formatCtx = nullptr; }
        }

    public:
        VideoPlayer() : sprite(texture)
        {
        }

        ~VideoPlayer()
        {
            cleanup();
        }

        bool load(const std::string& filename)
        {
            cleanup();
            isFinished = false;

            if (avformat_open_input(&formatCtx, filename.c_str(), nullptr, nullptr) != 0)
            {
                std::cerr << "[VIDEO ERROR] Nie udalo sie otworzyc pliku: " << filename << '\n';
                return false;
            }

            avformat_find_stream_info(formatCtx, nullptr);

            const AVCodec* codec = nullptr;
            for (unsigned int i = 0; i < formatCtx->nb_streams; i++)
            {
                if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
                {
                    videoStreamIndex = i;
                    codec = avcodec_find_decoder(formatCtx->streams[i]->codecpar->codec_id);
                    break;
                }
            }

            if (videoStreamIndex == -1 || !codec)
            {
                std::cerr << "[VIDEO ERROR] Brak strumienia wideo lub dekodera.\n";
                return false;
            }

            codecCtx = avcodec_alloc_context3(codec);
            avcodec_parameters_to_context(codecCtx, formatCtx->streams[videoStreamIndex]->codecpar);

            if (avcodec_open2(codecCtx, codec, nullptr) < 0)
            {
                std::cerr << "[VIDEO ERROR] Nie udalo sie otworzyc kodeka.\n";
                return false;
            }

            frame = av_frame_alloc();
            frameRGBA = av_frame_alloc();
            packet = av_packet_alloc();

            unsigned int width = codecCtx->width;
            unsigned int height = codecCtx->height;

            if (!texture.resize({ width, height }))
            {
                std::cerr << "[VIDEO ERROR] Nie udalo sie zmienic rozmiaru tekstury.\n";
                return false;
            }

            sprite.setTexture(texture, true);

            int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, width, height, 1);
            pixelBuffer.resize(numBytes);

            av_image_fill_arrays(frameRGBA->data, frameRGBA->linesize, pixelBuffer.data(),
                AV_PIX_FMT_RGBA, width, height, 1);

            swsCtx = sws_getContext(width, height, codecCtx->pix_fmt,
                width, height, AV_PIX_FMT_RGBA,
                SWS_BILINEAR, nullptr, nullptr, nullptr);

            update();

            return true;
        }

        void update()
        {
            if (isFinished) return;

            while (av_read_frame(formatCtx, packet) >= 0)
            {
                if (packet->stream_index == videoStreamIndex)
                {
                    if (avcodec_send_packet(codecCtx, packet) == 0)
                    {
                        if (avcodec_receive_frame(codecCtx, frame) == 0)
                        {
                            sws_scale(swsCtx, frame->data, frame->linesize, 0, codecCtx->height,
                                frameRGBA->data, frameRGBA->linesize);

                            texture.update(pixelBuffer.data());
                            av_packet_unref(packet);
                            return;
                        }
                    }
                }
                av_packet_unref(packet);
            }

            isFinished = true;
        }

        void fitToView(const sf::Vector2f& targetSize)
        {
            if (texture.getSize().x == 0) return;

            sf::Vector2f videoSize(texture.getSize());
            float scaleX = targetSize.x / videoSize.x;
            float scaleY = targetSize.y / videoSize.y;

            sprite.setScale({ scaleX, scaleY });
            sprite.setPosition({ 0.f, 0.f });
            sprite.setOrigin({ 0.f, 0.f });
        }

        const sf::Sprite& getSprite() const { return sprite; }
        bool isDone() const { return isFinished; }
    };
}