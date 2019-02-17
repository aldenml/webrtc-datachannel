#!/bin/bash

usage() { echo "Usage: $0 -p <target_os> -a <target_cpu>" 1>&2; exit 1; }

while getopts ":p:a:" o; do
    case "${o}" in
        p)
            TARGET_OS=${OPTARG}
            ;;
        a)
            TARGET_CPU=${OPTARG}
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

if [[ -z "${TARGET_OS}" ]] || [[ -z "${TARGET_CPU}" ]]; then
    usage
fi

echo "TARGET_OS = ${TARGET_OS}"
echo "TARGET_CPU = ${TARGET_CPU}"

case ${TARGET_OS} in
    mac)
        FETCH_CONFIG=webrtc
        GN_EXTRA_ARGS='use_custom_libcxx=false'
        ;;
    linux)
        FETCH_CONFIG=webrtc
        GN_EXTRA_ARGS='is_clang=false use_custom_libcxx=false treat_warnings_as_errors=false rtc_use_gtk=false use_ozone=true'
        ;;
#    android)
#        FETCH_CONFIG=webrtc_android
#        GN_EXTRA_ARGS='disable_android_lint=true'
#        ;;
    *)
        echo "Target OS not supported: ${TARGET_OS}"; exit 1;
        ;;
esac

echo "FETCH_CONFIG = ${FETCH_CONFIG}"

GN_COMMON_ARGS='use_rtti=true is_debug=false rtc_build_examples=false rtc_build_tools=false rtc_enable_protobuf=false rtc_include_ilbc=false rtc_include_opus=false rtc_use_h264=false rtc_include_pulse_audio=false'
GN_ARGS="${GN_COMMON_ARGS} ${GN_EXTRA_ARGS}"

echo "GN_ARGS = ${GN_ARGS}"

set -e

set -v

git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
export PATH=$PWD/depot_tools:$PATH

fetch --nohooks --no-history ${FETCH_CONFIG}

gclient sync --nohooks --no-history --shallow --with_branch_heads

sed -i -e "s|'src/resources'],|'src/resources'],'condition':'rtc_include_tests==true',|" src/DEPS

gclient sync --no-history --shallow --with_branch_heads
#git branch -v --all

cd src

git checkout DEPS

git checkout branch-heads/m73

gn gen out/Release --args='target_os='"\"${TARGET_OS}"\"' target_cpu='"\"${TARGET_CPU}"\"' '"${GN_ARGS}"

ninja -C out/Release webrtc

cd ..
