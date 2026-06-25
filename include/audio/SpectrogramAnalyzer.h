#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>
#include <vector>

#include "MelFilterbank.h"
#include "SpectrogramBuffer.h"

class SpectrogramAnalyzer {
public:
    SpectrogramAnalyzer(int frequencyBinCount, int melBandCount)
        : frequencyBinCount(frequencyBinCount),
          melBandCount(melBandCount),
          linearSpectrogram(frequencyBinCount),
          melSpectrogram(melBandCount),
          normalizedLinearFrame(frequencyBinCount, 0.0f),
          melBandFrame(melBandCount, 0.0f) {}

    void prepare(double sampleRate, int fftSize) {
        melFilterbank.prepare(sampleRate, fftSize, frequencyBinCount, melBandCount);
    }

    void pushFrame(const float* linearMagnitude) {
        const float gain = gainDecibels.load(std::memory_order_relaxed);

        for (int bin = 0; bin < frequencyBinCount; ++bin)
            normalizedLinearFrame[bin] = magnitudeToNormalizedLevel(linearMagnitude[bin], gain);
        linearSpectrogram.pushFrame(normalizedLinearFrame.data());

        melFilterbank.apply(linearMagnitude, melBandFrame.data());
        for (int band = 0; band < melBandCount; ++band)
            melBandFrame[band] = magnitudeToNormalizedLevel(melBandFrame[band], gain);
        melSpectrogram.pushFrame(melBandFrame.data());
    }

    void clear() {
        linearSpectrogram.clear();
        melSpectrogram.clear();
    }

    void preFill() {
        linearSpectrogram.preFill();
        melSpectrogram.preFill();
    }

    void setGainDecibels(float gain) {
        gainDecibels.store(gain, std::memory_order_relaxed);
    }

    const SpectrogramBuffer& getLinearSpectrogram() const { return linearSpectrogram; }
    const SpectrogramBuffer& getMelSpectrogram() const { return melSpectrogram; }

private:
    static constexpr float minimumDecibels = -80.0f;
    static constexpr float maximumDecibels =   0.0f;

    static float magnitudeToNormalizedLevel(float magnitude, float gain) {
        const float decibels = 20.0f * std::log10(std::max(magnitude, 1e-6f)) + gain;
        return std::clamp((decibels - minimumDecibels) / (maximumDecibels - minimumDecibels), 0.0f, 1.0f);
    }

    int frequencyBinCount;
    int melBandCount;

    MelFilterbank melFilterbank;
    SpectrogramBuffer linearSpectrogram;
    SpectrogramBuffer melSpectrogram;

    std::vector<float> normalizedLinearFrame;
    std::vector<float> melBandFrame;

    std::atomic<float> gainDecibels{0.0f};
};