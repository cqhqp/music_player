cmake_minimum_required(VERSION 3.14)
project(runner LANGUAGES CXX)

# Define the application target. To change its name, change BINARY_NAME in the
# top-level CMakeLists.txt, not the value here, or `flutter run` will no longer
# work.
#
# Any new source files that you add to the application should be added here.
add_executable(${BINARY_NAME} WIN32
  "flutter_window.cpp"
  "main.cpp"
  "utils.cpp"
  "win32_window.cpp"
  "${CMAKE_SOURCE_DIR}/module/audio_plugin_handle.cpp"
  "${CMAKE_SOURCE_DIR}/module/audio_manager.cpp"
  "${CMAKE_SOURCE_DIR}/module/audio_metadata.cpp"
  "${CMAKE_SOURCE_DIR}/module/audio_decoder.cpp"
  "${CMAKE_SOURCE_DIR}/module/audio_cache.cpp"
  "${FLUTTER_MANAGED_DIR}/generated_plugin_registrant.cc"
  "Runner.rc"
  "runner.exe.manifest"
)

# Apply the standard set of build settings. This can be removed for applications
# that need different build settings.
apply_standard_settings(${BINARY_NAME})

# Add preprocessor definitions for the build version.
target_compile_definitions(${BINARY_NAME} PRIVATE "FLUTTER_VERSION=\"${FLUTTER_VERSION}\"")
target_compile_definitions(${BINARY_NAME} PRIVATE "FLUTTER_VERSION_MAJOR=${FLUTTER_VERSION_MAJOR}")
target_compile_definitions(${BINARY_NAME} PRIVATE "FLUTTER_VERSION_MINOR=${FLUTTER_VERSION_MINOR}")
target_compile_definitions(${BINARY_NAME} PRIVATE "FLUTTER_VERSION_PATCH=${FLUTTER_VERSION_PATCH}")
target_compile_definitions(${BINARY_NAME} PRIVATE "FLUTTER_VERSION_BUILD=${FLUTTER_VERSION_BUILD}")

# Disable Windows macros that collide with C++ standard library functions.
target_compile_definitions(${BINARY_NAME} PRIVATE "NOMINMAX")

# Add dependency libraries and include directories. Add any application-specific
# dependencies here.
link_directories("${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/lib")  
link_directories("${CMAKE_SOURCE_DIR}/thridparty/glog/lib")  

target_link_libraries(${BINARY_NAME} PRIVATE
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/lib/avcodec.lib"
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/lib/avdevice.lib"
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/lib/avfilter.lib"
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/lib/avformat.lib"
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/lib/avutil.lib"
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/lib/postproc.lib"
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/lib/swresample.lib"
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/lib/swscale.lib"
  "${CMAKE_SOURCE_DIR}/thridparty/glog/lib/glogd.lib"
)

target_link_libraries(${BINARY_NAME} PRIVATE flutter flutter_wrapper_app)
target_link_libraries(${BINARY_NAME} PRIVATE "dwmapi.lib")
target_include_directories(${BINARY_NAME} PRIVATE 
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/module"
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/include"
  "${CMAKE_SOURCE_DIR}/thridparty/glog/include"
)

# List of absolute paths to libraries that should be bundled with the plugin
set(flutter_thridparty/ffmpeg_bundled_libraries
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/bin/avcodec-60.dll"
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/bin/avdevice-60.dll"
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/bin/avfilter-9.dll"
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/bin/avformat-60.dll"
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/bin/avutil-58.dll"
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/lib/postproc-57.dll"
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/bin/swresample-4.dll"
  "${CMAKE_SOURCE_DIR}/thridparty/ffmpeg/bin/swscale-7.dll"
  PARENT_SCOPE
)

# Run the Flutter tool portions of the build. This must not be removed.
add_dependencies(${BINARY_NAME} flutter_assemble)
