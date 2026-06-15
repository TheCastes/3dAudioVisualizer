#pragma once

#include <string>
#include <thread>

#include "../audio/AudioEngine.h"
#include "../renderer/Renderer.h"
#include "../ui/Ui.h"

class Application {
public:
    Application();
    ~Application();

    Application(const Application& copy) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&& move) = delete;
    Application& operator=(Application&&) = delete;

    int run();


private:
    void startJuceAudioThread();
    void waitUntilAudioIsReady();
    void runRenderLoop();

    Renderer renderer;
    Ui ui;
    AudioEngine audioEngine;
    std::thread juceAudioThread;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float currentFrame = 0.0;
};
