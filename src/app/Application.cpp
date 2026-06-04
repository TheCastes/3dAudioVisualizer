#include <juce_core/juce_core.h>
#include <chrono>
#include <iostream>
#include <string>

#include "../../include/app/Application.h"
#include "../../include/audio/SpectrogramExporter.h"

#include "juce_events/juce_events.h"
Application::Application() = default;

Application::~Application() {
    if (juceAudioThread.joinable()){
        juce::MessageManager::getInstance()->stopDispatchLoop();
        juceAudioThread.join();
    }
}


int Application::run(const std::string audioFileToPlay) {
    startJuceAudioThread(audioFileToPlay);
    waitUntilAudioIsReady();

    if (!renderer.init()) {
        std::cerr << "Renderer init fallito\n";
        return -1;
    }


    runRenderLoop();

    std::cout << "Chiusura in corso...\n";
    exportSpectrogram(audioFileToPlay);
    std::cout << "Spettrogramma esportato.\n";
    return 0;

}


void Application::startJuceAudioThread(const std::string& audioFileToPlay) {
    juceAudioThread = std::thread( [this, audioFileToPlay]() {

        juce::ScopedJuceInitialiser_GUI juceInit;
        juce::AudioDeviceManager deviceManager;

        auto initError = deviceManager.initialise(0, 2, nullptr, true);

        if(!initError.isEmpty()) {
            std::cerr << "JUCE Audio init skipped: " << initError.toStdString() << "\n";
            return;
        }

        deviceManager.addAudioCallback(&audioEngine);
        deviceManager.restartLastAudioDevice();

        if (audioEngine.loadFile(audioFileToPlay)) {
            audioEngine.play();
            std::cout << "Riproduzione: " << audioFileToPlay << "\n";
        } else {
            std::cerr << "Impossibile caricare: " << audioFileToPlay << "\n";
        }

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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        const float currentAudioLevel = audioEngine.getCurrentAudioLevel();
        renderer.render(currentAudioLevel);
    }
}


void Application::exportSpectrogram(const std::string& audioFileToPlay) {
    SpectrogramExporter::exportPPM(audioEngine.getSpectrogramBuffer(), "spectrogram_realtime.ppm");

    SpectrogramExporter::exportFullTrack(audioFileToPlay, "spectrogram_full.ppm");
}
