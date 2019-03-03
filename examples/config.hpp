/*
Copyright (c) 2019 Alden Torres
All rights reserved.

Licensed under the terms of the MIT license.
*/

#ifndef WEBRTC_DATACHANNEL_EXAMPLE_CONFIG_HPP
#define WEBRTC_DATACHANNEL_EXAMPLE_CONFIG_HPP

#if defined __APPLE__

#define WEBRTC_POSIX
#define WEBRTC_MAC

#elif defined __linux__

#define WEBRTC_POSIX
#define WEBRTC_LINUX

#elif defined __ANDROID__

#define WEBRTC_POSIX
#define WEBRTC_LINUX
#define WEBRTC_ANDROID

#elif defined _WIN32

#define WEBRTC_WIN

#else

#error "Unsupported platform"

#endif

#endif // WEBRTC_DATACHANNEL_EXAMPLE_CONFIG_HPP
