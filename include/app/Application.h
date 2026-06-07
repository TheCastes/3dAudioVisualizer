#pragma once

#include <string>
#include <thread>

#include "../audio/AudioEngine.h"
#include "../renderer/Renderer.h"
#include "../flocking/Flocking.h"

class Application {
public:
    Application();
    ~Application();

    Application(const Application& copy) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&& move) = delete;
    Application& operator=(Application&&) = delete;

    int run(const std::string audioFileToPlay);


private:
    void startJuceAudioThread(const std::string& audioFileToPlay);
    void waitUntilAudioIsReady();
    void runRenderLoop();
    void exportSpectrogram(const std::string& audioFileToPlay);

    Renderer renderer;
    AudioEngine audioEngine;
    std::thread juceAudioThread;

    Flocking flocking;
};
