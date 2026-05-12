#pragma once

#include <juce_dsp/juce_dsp.h>
#include "SpectrogramBuffer.h"

class STFTProcessor {
public:
    static constexpr int kFftOrder = 10;
    static constexpr int kFftSize  = 1 << kFftOrder; // 1024
    static constexpr int kHopSize  = kFftSize / 2;   // 512  (50 % overlap)
    static constexpr int kNumBins  = kFftSize / 2;   // 512  (positive freqs)

    static_assert(kNumBins == SpectrogramBuffer::kNumBins,
                  "STFTProcessor::kNumBins must match SpectrogramBuffer::kNumBins");

    explicit STFTProcessor(SpectrogramBuffer& sink);

    void pushSamples(const float* mono, int numSamples);

private:
    void processFrame();

    juce::dsp::FFT  fft_;
    SpectrogramBuffer& sink_;

    float inputRing_[kFftSize]{};
    float fftBuf_[kFftSize * 2]{};
    float window_[kFftSize]{};

    int ringPos_  = 0;
    int hopAccum_ = 0;
};
