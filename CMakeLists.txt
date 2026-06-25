cmake_minimum_required(VERSION 3.20)
project(3dAudioVisualizer LANGUAGES C CXX)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "Build in-source not enabled: make a separate build /folder and build inside it")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
endif()

if(NOT MSVC)
    find_program(MOLD_PROGRAM mold)
    find_program(LLD_PROGRAM ld.lld)
    if(MOLD_PROGRAM)
        add_link_options(-fuse-ld=mold)
        message(STATUS "Using linker: mold")
    elseif(LLD_PROGRAM)
        add_link_options(-fuse-ld=lld)
        message(STATUS "Using linker: lld")
    endif()
endif()

if(MSVC)
    add_compile_options(/utf-8 /EHsc /MP)
    add_compile_definitions(
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        _USE_MATH_DEFINES
        _CRT_SECURE_NO_WARNINGS
    )
endif()

include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.0
    GIT_SHALLOW TRUE
    GIT_SUBMODULES ""
)
FetchContent_MakeAvailable(juce)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.5
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    message(STATUS "glfw3 not found on system: building it via FetchContent")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glfw)
endif()

find_package(glm QUIET)
if(NOT glm_FOUND)
    message(STATUS "glm not found on system: fetching it via FetchContent")
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glm)
endif()

find_package(OpenGL REQUIRED)

add_library(glad STATIC include/glad/src/glad.c)
target_include_directories(glad PUBLIC include/glad/include)
target_link_libraries(glad PRIVATE ${CMAKE_DL_LIBS})

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

file(GLOB_RECURSE INCLUDE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/app/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/audio/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/renderer/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ui/*.h"
)

set(SOURCE_FILES
    src/main.cpp
    src/app/Application.cpp
    src/audio/AudioEngine.cpp
    src/renderer/Renderer.cpp
    src/renderer/Mesh.cpp
    src/renderer/GridMesh.cpp
    src/renderer/SphereFieldMesh.cpp
    src/renderer/SpectrogramTexture.cpp
    src/renderer/Trackball.cpp
    src/ui/Ui.cpp
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
    juce::juce_gui_basics
    juce::juce_core
    glfw
    glm::glm
    glad
    imgui
)
