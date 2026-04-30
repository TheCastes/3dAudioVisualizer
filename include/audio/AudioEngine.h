#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <atomic>
#include <cmath>

class AudioEngine : public juce::AudioIODeviceCallback {
public:
    double sr = 44100.0;
    double phase = 0.0;

    void audioDeviceAboutToStart(juce::AudioIODevice* d) override;
    void audioDeviceStopped() override;

    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int numInputChannels,
                                          float* const* outputChannelData, int numOutputChannels,
                                          int numSamples, const juce::AudioIODeviceCallbackContext&) override;

    static std::atomic<float>& getAudioLevel();
    static std::atomic<bool>& getAudioReady();
    static void runInBackground();
};