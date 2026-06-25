#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <chrono>
#include <iostream>
#include <string>

#include "app/Application.h"

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
        std::cerr << "Renderer init failed\n";
        return -1;
    }

    if (!ui.init(renderer.getWindow())) {
        std::cerr << "UI init failed\n";
        return -1;
    }

    runUiCallbacks();
    runRenderLoop();

    return 0;
}


void Application::openFileDialog() {
    juce::MessageManager::callAsync([this]() {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select a track",
            juce::File(),
            "*.wav;*.mp3;*.flac;*.m4a;*.ogg;*.aiff");

        constexpr int flags =
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

        fileChooser->launchAsync(flags, [this](const juce::FileChooser& chooser) {
            const juce::File file = chooser.getResult();
            if (file.existsAsFile())
                audioEngine.load(file.getFullPathName().toStdString());
        });
    });
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
        std::cout << "JUCE Audio initialized\n";
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
        renderer.pollEvents();

        ui.beginFrame();
        ui.draw({
            audioEngine.getCurrentTrackName(),
            audioEngine.isPlaying(),
            audioEngine.getPositionSeconds(),
            audioEngine.getLengthSeconds()
        }, renderer.getRenderMode(), renderer.getSpectrogramScale(), renderer.getShaderControls());

        renderer.render(audioEngine.getLinearSpectrogram(), audioEngine.getMelSpectrogram());
        ui.render();

        renderer.swapBuffers();
    }
}

void Application::runUiCallbacks() {
    ui.setBrowseCallback([this]() { openFileDialog(); });
    ui.setPlayPauseCallback([this]() { audioEngine.togglePlayback(); });
    ui.setEjectCallback([this]() { audioEngine.eject(); });
    ui.setRenderModeCallback([this](RenderMode mode) { renderer.setRenderMode(mode); });
    ui.setShaderCallback([this](int shaderIndex) { renderer.setActiveShader(shaderIndex); });
    ui.setColormapCallback([this](int colormapIndex) { renderer.setActiveColormap(colormapIndex); });
    ui.setSpectrogramScaleCallback([this](SpectrogramScale scale) { renderer.setSpectrogramScale(scale); });
    ui.setSpectrogramGainCallback([this](float gain) { audioEngine.setSpectrogramGainDecibels(gain); });
    ui.setResetRotationCallback([this]() { renderer.resetRotation(); });
    ui.setResetParametersCallback([this]() { renderer.resetParameters(); });
    ui.setPresetIsometricCallback([this]() { renderer.setPresetIsometric(); });
    ui.setPresetSpectrumCallback([this]() { renderer.setPresetSpectrum(); });
}