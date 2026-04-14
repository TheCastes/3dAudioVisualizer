cmake_minimum_required(VERSION 3.20)
project(3dAudioVisualizer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Utile per IDE/LSP

# 1. Fetch JUCE dinamicamente (senza submoduli per velocizzare)
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.0   # Versione stabile attuale
    GIT_SHALLOW    TRUE
    GIT_SUBMODULES ""
)
FetchContent_MakeAvailable(juce)

# 2. Trova librerie installate a sistema
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_library(glad STATIC include/glad/src/glad.c)
target_include_directories(glad PUBLIC include/glad/include)
target_link_libraries(glad PRIVATE ${CMAKE_DL_LIBS})

# 3. Target eseguibile
add_executable(3dAudioVisualizer src/main.cpp)

add_compile_definitions(JUCE_USE_CURL=0)

# Link librerie
target_link_libraries(3dAudioVisualizer PRIVATE 
    juce::juce_audio_devices
    juce::juce_core
    glfw
    glm::glm
    glad
)
