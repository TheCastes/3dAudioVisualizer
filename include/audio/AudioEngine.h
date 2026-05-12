#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "SpectrogramBuffer.h"
#include "STFTProcessor.h"

class AudioEngine : public juce::AudioIODeviceCallback {
public:
    AudioEngine();
    ~AudioEngine() override;

    bool loadFile(const std::string& path);

    void play();
    void stop();
    bool isPlaying() const;

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceIOCallbackWithContext(
        const float* const* inputChannelData, int numInputChannels,
        float* const* outputChannelData, int numOutputChannels,
        int numSamples, const juce::AudioIODeviceCallbackContext&) override;

    static std::atomic<float>& getAudioLevel();
    static std::atomic<bool>&  getAudioReady();
    static SpectrogramBuffer&  getSpectrogramBuffer();

private:
    double currentSampleRate = 44100.0;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    static std::atomic<float> currentAudioLevel;
    static std::atomic<bool>  audioReadyFlag;
    static SpectrogramBuffer  spectrogramFrameBuffer;

    STFTProcessor<SpectrogramBuffer> stftProcessor{ spectrogramFrameBuffer };
    std::vector<float> monoMixBuffer;
};
