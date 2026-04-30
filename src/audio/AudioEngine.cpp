#include "../../include/audio/AudioEngine.h"

std::atomic<float> g_audioLevel{0.0f};
std::atomic<bool> g_audioReady{false};

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* d) {
    sr = d->getCurrentSampleRate();
}

void AudioEngine::audioDeviceStopped() {}

void AudioEngine::audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int numInputChannels,
                                                    float* const* outputChannelData, int numOutputChannels,
                                                    int numSamples, const juce::AudioIODeviceCallbackContext&) {
    float sum = 0.0f;
    for (int i = 0; i < numSamples; ++i) {
        double s = std::sin(phase);
        phase += 2.0 * juce::MathConstants<double>::pi * 440.0 / sr;
        float v = static_cast<float>(s) * 0.1f;
        sum += v * v;

        if (numOutputChannels > 0 && outputChannelData[0]) outputChannelData[0][i] = v;
        if (numOutputChannels > 1 && outputChannelData[1]) outputChannelData[1][i] = v;
    }
    g_audioLevel.store(std::sqrt(sum / numSamples), std::memory_order_relaxed);
}

std::atomic<float>& AudioEngine::getAudioLevel() { 
    return g_audioLevel; 
}
std::atomic<bool>& AudioEngine::getAudioReady() {
    return g_audioReady; 
}

void AudioEngine::runInBackground() {
    juce::ScopedJuceInitialiser_GUI juceInit;
    AudioEngine eng;
    juce::AudioDeviceManager dm;

    auto err = dm.initialise(0, 2, nullptr, true);
    if (err.isEmpty()) {
        dm.addAudioCallback(&eng);
        dm.restartLastAudioDevice();
        g_audioReady.store(true, std::memory_order_release);
        std::cout << "JUCE Audio inizializzato\n";
        juce::MessageManager::getInstance()->runDispatchLoop();
    } else {
        std::cerr << "JUCE Audio init skipped: " << err.toStdString() << "\n";
    }
}
