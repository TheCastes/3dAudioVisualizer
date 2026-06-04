cmake_minimum_required(VERSION 3.20)
project(3dAudioVisualizer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.0
    GIT_SHALLOW TRUE
    GIT_SUBMODULES ""
)
FetchContent_MakeAvailable(juce)

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_library(glad STATIC include/glad/src/glad.c)
target_include_directories(glad PUBLIC include/glad/include)
target_link_libraries(glad PRIVATE ${CMAKE_DL_LIBS})

set(INCLUDE_FILES
    include/app/Application.h
    include/audio/AudioEngine.h
    include/audio/SpectrogramBuffer.h
    include/audio/STFTProcessor.h
    include/audio/SpectrogramExporter.h
    include/renderer/Renderer.h
    include/renderer/Mesh.h
    include/renderer/Shader.h
    include/renderer/Trackball.h
)

set(SOURCE_FILES
    src/main.cpp
    src/app/Application.cpp
    src/audio/AudioEngine.cpp
    src/audio/SpectrogramExporter.cpp
    src/renderer/Renderer.cpp
    src/renderer/Mesh.cpp
    src/renderer/Trackball.cpp
)

add_executable(3dAudioVisualizer
    ${SOURCE_FILES}
    ${INCLUDE_FILES}
)

add_compile_definitions(
    JUCE_USE_CURL=0
    JUCE_USE_MP3AUDIOFORMAT=1
    JUCE_DSP_USE_VDSP=0
)

target_link_libraries(3dAudioVisualizer PRIVATE
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_dsp
    juce::juce_core
    glfw
    glm::glm
    glad
)
