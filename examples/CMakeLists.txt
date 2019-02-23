project(webrtc-datachannel-examples)

add_executable(loopback_test
    config
    codec_factory
    loopback_test)

target_link_libraries(loopback_test PRIVATE webrtc-datachannel)

if (WIN32)
    target_compile_options(loopback_test PRIVATE /MT)
endif()

if (UNIX AND NOT APPLE)
    target_link_libraries(loopback_test PRIVATE ${CMAKE_DL_LIBS})
endif()
