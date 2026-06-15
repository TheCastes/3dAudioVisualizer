#include <juce_core/juce_core.h>
#include <chrono>
#include <iostream>
#include <string>

#include "../../include/app/Application.h"

#include "juce_events/juce_events.h"
Application::Application() = default;

Application::~Application() {
    if (juceAudioThread.joinable()){
        juce::MessageManager::getInstance()->stopDispatchLoop();
        juceAudioThread.join();
    }
}


int Application::run() {
    startJuceAudioThread();
    waitUntilAudioIsReady();

    if (!renderer.init()) {
        std::cerr << "Renderer init fallito\n";
        return -1;
    }

    if (!ui.init(renderer.getWindow())) {
        std::cerr << "UI init fallito\n";
        return -1;
    }

    ui.setLoadCallback([this](const std::string& path) { audioEngine.requestLoad(path); });
    ui.scanTracks("../assets/tracks");

    runRenderLoop();
    return 0;
}


void Application::startJuceAudioThread() {
    juceAudioThread = std::thread( [this]() {

        juce::ScopedJuceInitialiser_GUI juceInit;
        juce::AudioDeviceManager deviceManager;

        auto initError = deviceManager.initialise(0, 2, nullptr, true);

        if(!initError.isEmpty()) {
            std::cerr << "JUCE Audio init skipped: " << initError.toStdString() << "\n";
            return;
        }

        deviceManager.addAudioCallback(&audioEngine);
        deviceManager.restartLastAudioDevice();
        
        audioEngine.setAudioReady();
        std::cout << "JUCE Audio inizializzato\n";
        juce::MessageManager::getInstance()->runDispatchLoop();

        }

    );

}


void Application::waitUntilAudioIsReady() {
    constexpr auto readinessTimeout = std::chrono::seconds(2);
    const bool becameReady = audioEngine.waitUntilReady(readinessTimeout);

    if (!becameReady)
        std::cerr << "Audio engine timed out.\n";

}


void Application::runRenderLoop() {
    while (!renderer.shouldClose()) {
        glfwPollEvents();
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        const float currentAudioLevel = audioEngine.getCurrentAudioLevel();

        ui.beginFrame();
        ui.draw();
        renderer.render(currentAudioLevel);
        ui.render();
        renderer.swapBuffers();
    }
}
