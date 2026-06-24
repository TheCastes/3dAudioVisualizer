#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
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

    ui.setBrowseCallback([this]() { openFileDialog(); });
    ui.setPlayPauseCallback([this]() { audioEngine.togglePlayback(); });
    ui.setStopCallback([this]() { audioEngine.eject(); });
    ui.setRenderModeCallback([this](RenderMode mode) { renderer.setRenderMode(mode); });
    ui.setShaderCallback([this](int shaderIndex) { renderer.setActiveShader(shaderIndex); });
    ui.setColormapCallback([this](int colormapIndex) { renderer.setActiveColormap(colormapIndex); });
    ui.setSpectrogramScaleCallback([this](SpectrogramScale scale) { renderer.setSpectrogramScale(scale); });
    ui.setSpectrogramGainCallback([this](float gain) { audioEngine.setSpectrogramGainDecibels(gain); });
    ui.setResetRotationCallback([this]() { renderer.resetRotation(); });
    ui.setResetParametersCallback([this]() { renderer.resetParameters(); });

    runRenderLoop();
    return 0;
}


void Application::openFileDialog() {
    juce::MessageManager::callAsync([this]() {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Seleziona una traccia",
            juce::File(),
            "*.wav;*.mp3;*.flac;*.m4a;*.ogg;*.aiff");

        constexpr int flags =
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

        fileChooser->launchAsync(flags, [this](const juce::FileChooser& chooser) {
            const juce::File file = chooser.getResult();
            if (file.existsAsFile())
                audioEngine.requestLoad(file.getFullPathName().toStdString());
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
        ui.draw({
            audioEngine.getCurrentTrackName(),
            audioEngine.isPlaying(),
            audioEngine.getPositionSeconds(),
            audioEngine.getLengthSeconds()
        }, renderer.getRenderMode(), renderer.getSpectrogramScale(), renderer.shaderControls());
        renderer.render(currentAudioLevel, audioEngine.getLinearSpectrogram(), audioEngine.getMelSpectrogram());
        ui.render();

        renderer.swapBuffers();
    }
}
