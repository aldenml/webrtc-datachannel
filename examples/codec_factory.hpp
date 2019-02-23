/*
Copyright (c) 2019 Alden Torres
All rights reserved.

Licensed under the terms of the MIT license.
*/

#ifndef WEBRTC_DATACHANNEL_EXAMPLE_CODEC_FACTORY_HPP
#define WEBRTC_DATACHANNEL_EXAMPLE_CODEC_FACTORY_HPP

#include <api/audio_codecs/audio_encoder_factory.h>
#include <api/audio_codecs/audio_decoder_factory.h>
#include <api/video_codecs/video_encoder_factory.h>
#include <api/video_codecs/video_decoder_factory.h>

// to provide complete types
#include <api/video_codecs/video_encoder.h>
#include <api/video_codecs/video_decoder.h>

class EmptyAudioEncoderFactory : public webrtc::AudioEncoderFactory
{
    std::vector<webrtc::AudioCodecSpec> GetSupportedEncoders() override
    {
        return {};
    }

    absl::optional<webrtc::AudioCodecInfo> QueryAudioEncoder(
        webrtc::SdpAudioFormat const& format) override
    {
        return {};
    }

    std::unique_ptr<webrtc::AudioEncoder> MakeAudioEncoder(
        int payload_type
        , webrtc::SdpAudioFormat const& format
        , absl::optional<webrtc::AudioCodecPairId> codec_pair_id) override
    {
        return nullptr;
    }
};

class EmptyAudioDecoderFactory : public webrtc::AudioDecoderFactory
{
    std::vector<webrtc::AudioCodecSpec> GetSupportedDecoders() override
    {
        return {};
    }

    bool IsSupportedDecoder(webrtc::SdpAudioFormat const& format) override
    {
        return false;
    }

    std::unique_ptr<webrtc::AudioDecoder> MakeAudioDecoder(
        webrtc::SdpAudioFormat const& format
        , absl::optional<webrtc::AudioCodecPairId> codec_pair_id) override
    {
        return nullptr;
    }
};

class EmptyVideoEncoderFactory final : public webrtc::VideoEncoderFactory
{
    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override
    {
        return {};
    }

    CodecInfo QueryVideoEncoder(
        webrtc::SdpVideoFormat const& format) const override
    {
        return {};
    }

    std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(
        webrtc::SdpVideoFormat const& format) override
    {
        return nullptr;
    }
};

class EmptyVideoDecoderFactory final : public webrtc::VideoDecoderFactory
{
    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override
    {
        return {};
    }

    std::unique_ptr<webrtc::VideoDecoder> CreateVideoDecoder(
        webrtc::SdpVideoFormat const& format) override
    {
        return nullptr;
    }

    std::unique_ptr<webrtc::VideoDecoder> LegacyCreateVideoDecoder(
        webrtc::SdpVideoFormat const& format
        , std::string const& receive_stream_id) override
    {
        return nullptr;
    }
};

#endif // WEBRTC_DATACHANNEL_EXAMPLE_CODEC_FACTORY_HPP
