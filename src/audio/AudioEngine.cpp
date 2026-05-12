#include "../../include/audio/AudioEngine.h"
#include <cmath>

std::atomic<float> AudioEngine::currentAudioLevel{0.0f};
std::atomic<bool> AudioEngine::audioReadyFlag{false};
SpectrogramBuffer AudioEngine::spectrogramFrameBuffer{};

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

void AudioEngine::play() {
    transportSource.start();
}
void AudioEngine::stop() {
    transportSource.stop();
}
bool AudioEngine::isPlaying() const {
    return transportSource.isPlaying();
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device) {
    currentSampleRate = device->getCurrentSampleRate();
    const int blockSize = device->getCurrentBufferSizeSamples();
    transportSource.prepareToPlay(blockSize, currentSampleRate);
    monoMixBuffer.resize(blockSize, 0.0f);
}

void AudioEngine::audioDeviceStopped() {
    transportSource.releaseResources();
}

void AudioEngine::audioDeviceIOCallbackWithContext(const float* const*, int, float* const* outputChannelData, int numOutputChannels, int numSamples, const juce::AudioIODeviceCallbackContext&) {
    juce::AudioBuffer<float> buffer(outputChannelData, numOutputChannels, numSamples);
    juce::AudioSourceChannelInfo info(&buffer, 0, numSamples);
    transportSource.getNextAudioBlock(info);

    // RMS level
    float sumOfSquares = 0.0f;
    const int numChannels = buffer.getNumChannels();

    for (int channel = 0; channel < numChannels; ++channel) {
        const float* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < numSamples; ++sample)
            sumOfSquares += channelData[sample] * channelData[sample];
    }

    const int totalSampleCount = numSamples * numChannels;
    const float rmsLevel = totalSampleCount > 0 ? std::sqrt(sumOfSquares / totalSampleCount) : 0.0f;

    currentAudioLevel.store( rmsLevel, std::memory_order_relaxed );

    // Downmix to mono and feed the STFT
    if (monoMixBuffer.size() < numSamples)
        monoMixBuffer.resize(numSamples, 0.0f);

    const float channelScale = numChannels > 0 ? 1.0f / numChannels : 1.0f;
    std::fill(monoMixBuffer.begin(), monoMixBuffer.begin() + numSamples, 0.0f);

    for (int channel = 0; channel < numChannels; ++channel) {
        const float* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < numSamples; ++sample)
            monoMixBuffer[sample] += channelData[sample] * channelScale;
    }

    stftProcessor.pushSamples(monoMixBuffer.data(), numSamples);
}

std::atomic<float>& AudioEngine::getAudioLevel() {
    return currentAudioLevel;
}
std::atomic<bool>& AudioEngine::getAudioReady() {
    return audioReadyFlag;
}
SpectrogramBuffer& AudioEngine::getSpectrogramBuffer() {
    return spectrogramFrameBuffer;
}
