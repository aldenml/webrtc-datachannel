cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)

project(webrtc-datachannel)

option(BUILD_EXAMPLES "build examples" OFF)

add_library(webrtc-datachannel SHARED IMPORTED)

target_include_directories(webrtc-datachannel INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/third_party/abseil-cpp
)

if (WIN32)
    set_target_properties(webrtc-datachannel PROPERTIES
        IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/webrtc.lib
    )
else()
    set_target_properties(webrtc-datachannel PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/libwebrtc.a
    )
endif()

target_compile_features(webrtc-datachannel INTERFACE cxx_std_14)

if (WIN32)
    target_link_libraries(webrtc-datachannel INTERFACE winmm)
    target_link_libraries(webrtc-datachannel INTERFACE secur32)
    target_link_libraries(webrtc-datachannel INTERFACE wsock32)
    target_link_libraries(webrtc-datachannel INTERFACE ws2_32)
    target_link_libraries(webrtc-datachannel INTERFACE msdmo)
    target_link_libraries(webrtc-datachannel INTERFACE dmoguids)
    target_link_libraries(webrtc-datachannel INTERFACE wmcodecdspuuid)
endif()

if (APPLE)
    target_link_libraries(webrtc-datachannel INTERFACE "-framework Foundation")
    target_link_libraries(webrtc-datachannel INTERFACE "-framework CoreAudio")
    target_link_libraries(webrtc-datachannel INTERFACE "-framework AudioToolbox")
    target_link_libraries(webrtc-datachannel INTERFACE "-framework CoreGraphics")
endif()

if (UNIX AND NOT APPLE)
    find_package(Threads)
    target_link_libraries(webrtc-datachannel INTERFACE Threads::Threads)
endif()

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()
