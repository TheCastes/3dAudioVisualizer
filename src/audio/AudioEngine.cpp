#include "../../include/audio/AudioEngine.h"
#include <iostream>

std::atomic<float> AudioEngine::s_audioLevel{0.0f};
std::atomic<bool>  AudioEngine::s_audioReady{false};

AudioEngine::AudioEngine() {
    formatManager.registerBasicFormats();
    formatManager.registerFormat(new juce::MP3AudioFormat(), true);
}

AudioEngine::~AudioEngine() {
    transportSource.setSource(nullptr);
}

bool AudioEngine::loadFile(const std::string& path) {
    juce::File file(path);
    auto* reader = formatManager.createReaderFor(file);
    if (!reader) return false;

    auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
    transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
    readerSource = std::move(newSource);
    return true;
}

void AudioEngine::play() { transportSource.start(); }
void AudioEngine::stop() { transportSource.stop(); }
bool AudioEngine::isPlaying() const { return transportSource.isPlaying(); }

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device) {
    sampleRate = device->getCurrentSampleRate();
    transportSource.prepareToPlay(device->getCurrentBufferSizeSamples(), sampleRate);
}

void AudioEngine::audioDeviceStopped() {
    transportSource.releaseResources();
}

void AudioEngine::audioDeviceIOCallbackWithContext(
    const float* const*, int,
    float* const* outputChannelData, int numOutputChannels,
    int numSamples, const juce::AudioIODeviceCallbackContext&)
{
    juce::AudioBuffer<float> buffer(outputChannelData, numOutputChannels, numSamples);
    juce::AudioSourceChannelInfo info(&buffer, 0, numSamples);
    transportSource.getNextAudioBlock(info);

    float sum = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
            sum += data[i] * data[i];
    }
    const int totalSamples = numSamples * buffer.getNumChannels();
    s_audioLevel.store(totalSamples > 0 ? std::sqrt(sum / totalSamples) : 0.0f,
                       std::memory_order_relaxed);
}

std::atomic<float>& AudioEngine::getAudioLevel() { return s_audioLevel; }
std::atomic<bool>&  AudioEngine::getAudioReady() { return s_audioReady; }

void AudioEngine::runInBackground() {
    juce::ScopedJuceInitialiser_GUI juceInit;
    AudioEngine eng;
    juce::AudioDeviceManager dm;

    auto err = dm.initialise(0, 2, nullptr, true);
    if (err.isEmpty()) {
        dm.addAudioCallback(&eng);
        dm.restartLastAudioDevice();
        s_audioReady.store(true, std::memory_order_release);
        std::cout << "JUCE Audio inizializzato\n";
        juce::MessageManager::getInstance()->runDispatchLoop();
    } else {
        std::cerr << "JUCE Audio init skipped: " << err.toStdString() << "\n";
    }
}
