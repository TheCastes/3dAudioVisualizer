#pragma once
#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <cmath>

struct STFTParameters {
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder; // 1024
    static constexpr int hopSize = fftSize / 2;   // 512 (50% overlap)
    static constexpr int numFrequencyBins = fftSize / 2;   // 512 (positive freqs)
};

template<typename Sink>
class STFTProcessor : STFTParameters {
public:
    explicit STFTProcessor(Sink& sink) : fft(fftOrder), spectrogramSink(sink) {
        for (int i = 0; i < fftSize; ++i)
            hannWindow[i] = 0.5f * (1.0f - std::cos(juce::MathConstants<float>::twoPi * i / (fftSize - 1)));
    }

    void reset() {
        std::fill(inputRingBuffer, inputRingBuffer + fftSize, 0.0f);
        ringWritePosition = 0;
        samplesSinceLastHop = 0;
    }

    void pushSamples(const float* monoSamples, int numSamples) {
        for (int i = 0; i < numSamples; ++i) {
            inputRingBuffer[ringWritePosition] = monoSamples[i];
            ringWritePosition = (ringWritePosition + 1) % fftSize;
            if (++samplesSinceLastHop >= hopSize) {
                samplesSinceLastHop = 0;
                processFrame();
            }
        }
    }

private:
    juce::dsp::FFT fft;
    Sink& spectrogramSink;
    float inputRingBuffer[fftSize]{};
    float fftWorkBuffer[fftSize * 2]{};
    float hannWindow[fftSize]{};
    int ringWritePosition = 0;
    int samplesSinceLastHop = 0;

    void processFrame() {
        for (int i = 0; i < fftSize; ++i) {
            int sourceIndex = (ringWritePosition + i) % fftSize;
            fftWorkBuffer[i] = inputRingBuffer[sourceIndex] * hannWindow[i];
        }
        std::fill(fftWorkBuffer + fftSize, fftWorkBuffer + fftSize * 2, 0.0f);
        fft.performRealOnlyForwardTransform(fftWorkBuffer);

        float magnitudeBins[numFrequencyBins];
        for (int k = 0; k < numFrequencyBins; ++k) {
            float realPart  = fftWorkBuffer[2 * k];
            float imaginaryPart  = fftWorkBuffer[2 * k + 1];
            magnitudeBins[k] = std::sqrt(realPart * realPart + imaginaryPart * imaginaryPart);
        }
        spectrogramSink.pushFrame(magnitudeBins);
    }

};
