#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "SpectrogramBuffer.h"
#include "STFTProcessor.h"

class AudioEngine : public juce::AudioIODeviceCallback {
public:
    AudioEngine();
    ~AudioEngine() override;

    AudioEngine(const AudioEngine& copy) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;
    AudioEngine(AudioEngine&& move) = delete;
    AudioEngine& operator=(AudioEngine&&) = delete;

    bool loadFile(const std::string& path);

    void play();
    void stop();
    bool isPlaying() const;

    float getCurrentAudioLevel() const;

    bool isAudioReady() const;
    void setAudioReady();
    bool waitUntilReady(std::chrono::milliseconds timeout);

    SpectrogramBuffer& getSpectrogramBuffer();
    const SpectrogramBuffer& getSpectrogramBuffer() const;

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceIOCallbackWithContext(
        const float* const* inputChannelData, int numInputChannels,
        float* const* outputChannelData, int numOutputChannels,
        int numSamples, const juce::AudioIODeviceCallbackContext&) override;

private:
    double currentSampleRate = 44100.0;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    std::atomic<float> currentAudioLevel{0.0f};
    std::atomic<bool> audioReadyFlag{false};
    std::mutex audioReadyMutex;
    std::condition_variable audioReadyCondition;

    SpectrogramBuffer spectrogramFrameBuffer;

    STFTProcessor<SpectrogramBuffer> stftProcessor{ spectrogramFrameBuffer };
    std::vector<float> monoMixBuffer;
};
