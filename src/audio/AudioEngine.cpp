#include "../../include/audio/AudioEngine.h"
#include <cmath>

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

    {
        std::lock_guard<std::mutex> lock(trackNameMutex);
        currentTrackName = file.getFileName().toStdString();
    }

    spectrogramAnalyzer.preFill();
    return true;
}

void AudioEngine::requestLoad(const std::string& path) {
    juce::MessageManager::callAsync([this, path]() {
        if (loadFile(path)) {
            transportSource.setPosition(0.0);
            play();
        }
    });
}

void AudioEngine::play() {
    transportSource.start();
}
void AudioEngine::stop() {
    transportSource.stop();
}
void AudioEngine::togglePlayback() {
    juce::MessageManager::callAsync([this]() {
        if (readerSource == nullptr) return;
        if (transportSource.isPlaying())
            transportSource.stop();
        else
            transportSource.start();
    });
}
void AudioEngine::eject() {
    juce::MessageManager::callAsync([this]() {
        transportSource.stop();
        transportSource.setSource(nullptr);
        readerSource.reset();
        spectrogramAnalyzer.clear();
        stftProcessor.reset();
        {
            std::lock_guard<std::mutex> lock(trackNameMutex);
            currentTrackName.clear();
        }
    });
}
bool AudioEngine::isPlaying() const {
    return transportSource.isPlaying();
}

double AudioEngine::getPositionSeconds() const {
    return transportSource.getCurrentPosition();
}

double AudioEngine::getLengthSeconds() const {
    return transportSource.getLengthInSeconds();
}

std::string AudioEngine::getCurrentTrackName() const {
    std::lock_guard<std::mutex> lock(trackNameMutex);
    return currentTrackName;
}

float AudioEngine::getCurrentAudioLevel() const {
    return currentAudioLevel.load(std::memory_order_relaxed);
}

bool AudioEngine::isAudioReady() const {
    return audioReadyFlag.load(std::memory_order_acquire);
}

void AudioEngine::setAudioReady() {
    {
        std::lock_guard<std::mutex> lock(audioReadyMutex);
        audioReadyFlag.store(true, std::memory_order_release);
    }

    audioReadyCondition.notify_one();
}

bool AudioEngine::waitUntilReady(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(audioReadyMutex);

    return audioReadyCondition.wait_for(lock, timeout, [this]() {
            return audioReadyFlag.load(std::memory_order_acquire);
        }
    );
}

const SpectrogramBuffer& AudioEngine::getLinearSpectrogram() const {
    return spectrogramAnalyzer.getLinearSpectrogram();
}

const SpectrogramBuffer& AudioEngine::getMelSpectrogram() const {
    return spectrogramAnalyzer.getMelSpectrogram();
}

void AudioEngine::setSpectrogramGainDecibels(float gain) {
    spectrogramAnalyzer.setGainDecibels(gain);
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device) {
    currentSampleRate = device->getCurrentSampleRate();
    const int blockSize = device->getCurrentBufferSizeSamples();
    transportSource.prepareToPlay(blockSize, currentSampleRate);
    spectrogramAnalyzer.prepare(currentSampleRate, STFTProcessor<SpectrogramAnalyzer>::fftSize);
    monoMixBuffer.resize(blockSize, 0.0f);
}

void AudioEngine::audioDeviceStopped() {
    transportSource.releaseResources();
}

void AudioEngine::audioDeviceIOCallbackWithContext(const float* const*, int, float* const* outputChannelData, int numOutputChannels, int numSamples, const juce::AudioIODeviceCallbackContext&) {
    juce::AudioBuffer<float> buffer(outputChannelData, numOutputChannels, numSamples);
    
    if (!transportSource.isPlaying()) {
        buffer.clear();
        currentAudioLevel.store(0.0f, std::memory_order_relaxed);
        return;
    }

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
