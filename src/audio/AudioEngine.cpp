#include "../../include/audio/AudioEngine.h"
#include <iostream>

std::atomic<float>  AudioEngine::s_audioLevel{0.0f};
std::atomic<bool>   AudioEngine::s_audioReady{false};
SpectrogramBuffer   AudioEngine::s_spectrogramBuffer{};

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
    monoMix_.resize(device->getCurrentBufferSizeSamples(), 0.0f);
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

    // RMS level
    float sum = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
            sum += data[i] * data[i];
    }
    const int totalSamples = numSamples * buffer.getNumChannels();
    s_audioLevel.store(totalSamples > 0 ? std::sqrt(sum / totalSamples) : 0.0f,
                       std::memory_order_relaxed);

    // Downmix to mono and feed the STFT
    if ((int)monoMix_.size() < numSamples)
        monoMix_.resize(numSamples, 0.0f);

    const int numCh = buffer.getNumChannels();
    const float scale = numCh > 0 ? 1.0f / numCh : 1.0f;
    std::fill(monoMix_.begin(), monoMix_.begin() + numSamples, 0.0f);
    for (int ch = 0; ch < numCh; ++ch) {
        const float* src = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
            monoMix_[i] += src[i] * scale;
    }
    stft_.pushSamples(monoMix_.data(), numSamples);
}

std::atomic<float>&  AudioEngine::getAudioLevel()       { return s_audioLevel; }
std::atomic<bool>&   AudioEngine::getAudioReady()       { return s_audioReady; }
SpectrogramBuffer&   AudioEngine::getSpectrogramBuffer(){ return s_spectrogramBuffer; }

