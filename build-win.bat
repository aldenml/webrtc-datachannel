set DEPOT_TOOLS_WIN_TOOLCHAIN=0

call git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
set PATH=%CD%\depot_tools;%PATH%

call gclient
call where python

call fetch --nohooks --no-history webrtc

call gclient sync --nohooks --no-history --shallow --with_branch_heads

call sed -i -e "s|'src/resources'],|'src/resources'],'condition':'rtc_include_tests==true',|" src\DEPS

call gclient sync --no-history --shallow --with_branch_heads

cd src

call git checkout DEPS

call git checkout branch-heads/m73

call gn gen out/Release --args="use_rtti=true is_debug=false is_clang=false rtc_build_examples=false rtc_build_tools=false rtc_enable_protobuf=false rtc_include_ilbc=false rtc_include_opus=false rtc_use_h264=false"

call ninja -C out/Release webrtc

cd ..
