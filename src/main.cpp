#include <juce_core/juce_core.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "../include/audio/AudioEngine.h"
#include "../include/audio/SpectrogramExporter.h"
#include "../include/renderer/Renderer.h"

int main(int argc, char* argv[]) {
    const std::string audioFile = (argc > 1) ? argv[1] : "";

    AudioEngine audioEngine;

    std::thread juceThread([&audioEngine, &audioFile]() {
        juce::ScopedJuceInitialiser_GUI juceInit;
        juce::AudioDeviceManager deviceManager;

        auto initError = deviceManager.initialise(0, 2, nullptr, true);
        if (initError.isEmpty()) {
            deviceManager.addAudioCallback(&audioEngine);
            deviceManager.restartLastAudioDevice();

            if (!audioFile.empty()) {
                if (audioEngine.loadFile(audioFile)) {
                    audioEngine.play();
                    std::cout << "Riproduzione: " << audioFile << "\n";
                } else {
                    std::cerr << "Impossibile caricare: " << audioFile << "\n";
                }
            }

            audioEngine.setAudioReady();
            std::cout << "JUCE Audio inizializzato\n";
            juce::MessageManager::getInstance()->runDispatchLoop();
        } else {
            std::cerr << "JUCE Audio init skipped: " << initError.toStdString() << "\n";
        }
    });

    for (int waitIteration = 0; waitIteration < 200 && !audioEngine.isAudioReady(); ++waitIteration) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (!audioFile.empty())
        SpectrogramExporter::exportFullTrack(audioFile, "spectrogram_full.ppm");

    Renderer renderer;

    if (!renderer.init()) {
        std::cerr << "Renderer init fallito\n";
        juce::MessageManager::getInstance()->stopDispatchLoop();
        juceThread.join();
        return -1;
    }

    while (!renderer.shouldClose()) {
        glfwPollEvents();
        renderer.render(audioEngine.getCurrentAudioLevel());
    }

    std::cout << "Chiusura in corso...\n";

    SpectrogramExporter::exportPPM(audioEngine.getSpectrogramBuffer(), "spectrogram_realtime.ppm");

    juce::MessageManager::getInstance()->stopDispatchLoop();
    juceThread.join();

    std::cout << "Uscita pulita.\n";
    return 0;
}