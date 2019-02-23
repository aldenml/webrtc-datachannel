/*
Copyright (c) 2019 Alden Torres
All rights reserved.

Licensed under the terms of the MIT license.
*/

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <array>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "config.hpp"

#include <rtc_base/ssl_adapter.h>
#include <rtc_base/thread.h>

#include <api/peer_connection_interface.h>
#include <api/create_peerconnection_factory.h>

#include "codec_factory.hpp"

struct CSDObserver;
struct SSDObserver;

struct LoopbackPeer
    : public webrtc::PeerConnectionObserver
    , public webrtc::DataChannelObserver
{

    // webrtc::PeerConnectionObserver

    void OnSignalingChange(
        webrtc::PeerConnectionInterface::SignalingState new_state) override
    {
        // useful to debug what's the status while destroying connections
        //Log("SignalingState: " + std::to_string(new_state));
    }

    void OnDataChannel(
        rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override
    {
        Log("recv data channel created - " + data_channel->label());
        m_recv_dc = data_channel;
        m_recv_dc->RegisterObserver(this);

        if (m_recv_dc && m_other->m_recv_dc)
        {
            Log("both RECV channels are up, sending PING");

            webrtc::DataBuffer buf{"PING"};
            m_send_dc->Send(buf);
        }
    }

    void OnRenegotiationNeeded() override
    {}

    void OnIceConnectionChange(
        webrtc::PeerConnectionInterface::IceConnectionState new_state) override
    {}

    void OnIceGatheringChange(
        webrtc::PeerConnectionInterface::IceGatheringState new_state) override
    {}

    void OnIceCandidate(webrtc::IceCandidateInterface const* candidate) override
    {
        std::string sdp;
        candidate->ToString(&sdp);

        Log("got ICE - " + sdp);
        m_other->m_connection->AddIceCandidate(candidate);
    }

    // webrtc::DataChannelObserver

    void OnStateChange() override
    {}

    void OnMessage(webrtc::DataBuffer const& buffer) override
    {
        std::string data{reinterpret_cast<char const*>(buffer.data.data()), buffer.size()};
        Log("got MSG - " + data);

        m_cv.notify_one();
    }

    // LoopbackPeer

    void CreateDataChannel(std::string const& label)
    {
        webrtc::DataChannelInit config;
        m_send_dc = m_connection->CreateDataChannel(m_name + ":" + label, &config);
        m_send_dc->RegisterObserver(this);
    }

    void CreateOffer()
    {
        webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
        m_connection->CreateOffer(m_csd_observer, options);
    }

    void CreateAnswer()
    {
        webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
        m_connection->CreateAnswer(m_csd_observer, options);
    }

    void SetLocalDescription(webrtc::SessionDescriptionInterface* desc)
    {
        std::string sdp;
        desc->ToString(&sdp);

        // since SetLocalDescription takes ownership of the desc,
        // a new one is created
        auto newDesc = webrtc::CreateSessionDescription(desc->GetType(), sdp);
        m_connection->SetLocalDescription(m_ssd_observer, newDesc.release());
    }

    void SetRemoteDescription(webrtc::SessionDescriptionInterface* desc)
    {
        std::string sdp;
        desc->ToString(&sdp);

        // since SetLocalDescription takes ownership of the desc,
        // a new one is created
        auto newDesc = webrtc::CreateSessionDescription(desc->GetType(), sdp);
        m_connection->SetRemoteDescription(m_ssd_observer, newDesc.release());
    }

    // private
    void Log(std::string const& msg)
    {
        std::cout << m_name << ": " << msg << std::endl;
    }

    // private
    void SetupObservers();

    // private
    void CloseDataChannels()
    {
        if (m_send_dc)
        {
            m_send_dc->Close();
            m_send_dc->UnregisterObserver();
            m_send_dc = nullptr;
        }

        if (m_recv_dc)
        {
            m_recv_dc->Close();
            m_recv_dc->UnregisterObserver();
            m_recv_dc = nullptr;
        }
    }

    LoopbackPeer(std::string name, std::condition_variable& cv)
        : m_name{std::move(name)}
        , m_cv{cv}
    {
        SetupObservers();
    }

    ~LoopbackPeer() override
    {
        CloseDataChannels();

        m_connection->Close();
        m_connection = nullptr;

        m_csd_observer = nullptr;
        m_ssd_observer = nullptr;
    }

    std::string m_name;
    std::condition_variable& m_cv;

    rtc::scoped_refptr<webrtc::PeerConnectionInterface> m_connection;

    rtc::scoped_refptr<webrtc::CreateSessionDescriptionObserver> m_csd_observer;
    rtc::scoped_refptr<webrtc::SetSessionDescriptionObserver> m_ssd_observer;

    rtc::scoped_refptr<webrtc::DataChannelInterface> m_send_dc;
    rtc::scoped_refptr<webrtc::DataChannelInterface> m_recv_dc;

    // NOTE: quick solution for this limited test
    LoopbackPeer* m_other = nullptr;
};

struct CSDObserver : public webrtc::CreateSessionDescriptionObserver
{
    explicit CSDObserver(LoopbackPeer* peer)
        : m_peer{peer}
    {}

    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override
    {
        std::string sdp;
        desc->ToString(&sdp);
        m_peer->Log("got SDP\n" + sdp);

        m_peer->SetLocalDescription(desc);

        m_peer->m_other->SetRemoteDescription(desc);

        if (desc->GetType() == webrtc::SdpType::kOffer)
            m_peer->m_other->CreateAnswer();
    }

    void OnFailure(std::string const& error) override
    {}

    LoopbackPeer* m_peer;
};

struct SSDObserver : public webrtc::SetSessionDescriptionObserver
{
    void OnSuccess() override
    {}

    void OnFailure(std::string const& error) override
    {}
};

void LoopbackPeer::SetupObservers()
{
    m_csd_observer = new rtc::RefCountedObject<CSDObserver>(this);
    m_ssd_observer = new rtc::RefCountedObject<SSDObserver>();
}

auto default_stun_servers()
{
    std::array<std::string, 3> stun_servers = {
        "stun:stun.l.google.com:19302",
        "stun:stun1.l.google.com:19302",
        "stun:stun2.l.google.com:19305",
    };

    std::vector<webrtc::PeerConnectionInterface::IceServer> servers;

    for (auto const& s : stun_servers)
    {
        webrtc::PeerConnectionInterface::IceServer ice_server;
        ice_server.uri = s;
        servers.push_back(ice_server);
    }

    return servers;
}


std::unique_ptr<LoopbackPeer> create_peer(
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> const& factory
    , std::string name
    , std::condition_variable& cv)
{
    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.servers = default_stun_servers();

    // NOTE: the situation of weird construction logic is something
    // that needs to be worked out
    auto peer = std::make_unique<LoopbackPeer>(name, cv);

    webrtc::PeerConnectionDependencies dependencies{peer.get()};

    peer->m_connection = factory->CreatePeerConnection(
        config, std::move(dependencies));

    return peer;
}

/*
 This program only creates two local WebRTC connections and opens
 a data channel between them, once all channels are up, one PING
 is sent and the condition variable is set.
 */
int main(int argc, char* argv[])
{
    rtc::InitializeSSL();

    std::mutex m_mutex;
    std::condition_variable cv;

    auto signaling_thread = rtc::Thread::CreateWithSocketServer();

    signaling_thread->Start();

    auto audio_encoder_factory
        = new rtc::RefCountedObject<EmptyAudioEncoderFactory>();
    auto audio_decoder_factory
        = new rtc::RefCountedObject<EmptyAudioDecoderFactory>();

    auto video_encoder_factory = std::make_unique<EmptyVideoEncoderFactory>();
    auto video_decoder_factory = std::make_unique<EmptyVideoDecoderFactory>();

    auto factory = webrtc::CreatePeerConnectionFactory(
        nullptr /* network_thread */
        , nullptr /* worker_thread */
        , signaling_thread.get()
        , nullptr /* default_adm */
        , audio_encoder_factory
        , audio_decoder_factory
        , std::move(video_encoder_factory)
        , std::move(video_decoder_factory)
        , nullptr /* audio_mixer */
        , nullptr /* audio_processing */);

    auto p1 = create_peer(factory, "p1", cv);
    auto p2 = create_peer(factory, "p2", cv);

    p1->m_other = p2.get();
    p2->m_other = p1.get();

    // it is important to create the data channels before
    // creating the offer, otherwise you will not get the
    // necessary data info in the SDP
    p1->CreateDataChannel("data");
    p2->CreateDataChannel("data");

    p1->CreateOffer();

    std::unique_lock<std::mutex> lock(m_mutex);
    cv.wait(lock);

    p1 = nullptr;
    p2 = nullptr;

    factory = nullptr;

    audio_encoder_factory = nullptr;
    audio_decoder_factory = nullptr;

    signaling_thread->Stop();
    signaling_thread = nullptr;

    rtc::CleanupSSL();

    return 0;
}
