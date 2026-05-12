#include "../include/audio/AudioEngine.h"
#include "../include/audio/SpectrogramExporter.h"
#include "../include/renderer/Renderer.h"

#include <juce_core/juce_core.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    const std::string audioFile = (argc > 1) ? argv[1] : "";

    AudioEngine eng;

    std::thread juceThread([&eng, &audioFile]() {
        juce::ScopedJuceInitialiser_GUI juceInit;
        juce::AudioDeviceManager dm;

        auto err = dm.initialise(0, 2, nullptr, true);
        if (err.isEmpty()) {
            dm.addAudioCallback(&eng);
            dm.restartLastAudioDevice();

            if (!audioFile.empty()) {
                if (eng.loadFile(audioFile)) {
                    eng.play();
                    std::cout << "Riproduzione: " << audioFile << "\n";
                } else {
                    std::cerr << "Impossibile caricare: " << audioFile << "\n";
                }
            }

            AudioEngine::getAudioReady().store(true, std::memory_order_release);
            std::cout << "JUCE Audio inizializzato\n";
            juce::MessageManager::getInstance()->runDispatchLoop();
        } else {
            std::cerr << "JUCE Audio init skipped: " << err.toStdString() << "\n";
        }
    });

    for (int i = 0; i < 200 && !AudioEngine::getAudioReady().load(); ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    if (!audioFile.empty())
        SpectrogramExporter::exportFullTrack(audioFile, "spectrogram_full.ppm");

    if (!Renderer::init()) {
        std::cerr << "Renderer init fallito\n";
        return -1;
    }

    while (!glfwWindowShouldClose(Renderer::getWindow())) {
        glfwPollEvents();
        Renderer::render();
    }

    std::cout << "Chiusura in corso...\n";

    SpectrogramExporter::exportPPM(AudioEngine::getSpectrogramBuffer(), "spectrogram_realtime.ppm");

    Renderer::shutdown();
    juce::MessageManager::getInstance()->stopDispatchLoop();
    juceThread.join();

    std::cout << "Uscita pulita.\n";
    return 0;
}