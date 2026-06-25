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

#include "SpectrogramAnalyzer.h"
#include "STFTProcessor.h"

class AudioEngine : public juce::AudioIODeviceCallback {
public:
    AudioEngine();
    ~AudioEngine() override;

    AudioEngine(const AudioEngine& copy) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;
    AudioEngine(AudioEngine&& move) = delete;
    AudioEngine& operator=(AudioEngine&&) = delete;

    void load(const std::string& path);

    void togglePlayback();
    void eject();
    bool isPlaying() const;

    double getPositionSeconds() const;
    double getLengthSeconds() const;

    std::string getCurrentTrackName() const;

    void setAudioReady();
    bool waitUntilReady(std::chrono::milliseconds timeout);

    const SpectrogramBuffer& getLinearSpectrogram() const;
    const SpectrogramBuffer& getMelSpectrogram() const;

    void setSpectrogramGainDecibels(float gain);

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceIOCallbackWithContext(
        const float* const* inputChannelData, int numInputChannels,
        float* const* outputChannelData, int numOutputChannels,
        int numSamples, const juce::AudioIODeviceCallbackContext&) override;

private:
    bool loadFile(const std::string& path);

    double currentSampleRate = 44100.0;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    mutable std::mutex trackNameMutex;
    std::string currentTrackName;

    std::atomic<bool> audioReadyFlag{false};
    std::mutex audioReadyMutex;
    std::condition_variable audioReadyCondition;

    static constexpr int melBandCount = 128;

    SpectrogramAnalyzer spectrogramAnalyzer{ STFTParameters::numFrequencyBins, melBandCount };

    STFTProcessor<SpectrogramAnalyzer> stftProcessor{ spectrogramAnalyzer };
    std::vector<float> monoMixBuffer;
};
