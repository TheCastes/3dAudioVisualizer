#include "../../include/audio/STFTProcessor.h"

#include <algorithm>
#include <cmath>

static constexpr float kMinDb = -80.0f;
static constexpr float kMaxDb =   0.0f;

STFTProcessor::STFTProcessor(SpectrogramBuffer& sink)
    : fft_(kFftOrder), sink_(sink)
{
    for (int i = 0; i < kFftSize; ++i)
        window_[i] = 0.5f * (1.0f - std::cos(2.0f * float(M_PI) * i / (kFftSize - 1)));
}

void STFTProcessor::pushSamples(const float* mono, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        inputRing_[ringPos_] = mono[i];
        ringPos_ = (ringPos_ + 1) % kFftSize;
        ++hopAccum_;

        if (hopAccum_ >= kHopSize) {
            hopAccum_ = 0;
            processFrame();
        }
    }
}

void STFTProcessor::processFrame() {
    for (int i = 0; i < kFftSize; ++i) {
        int src = (ringPos_ + i) % kFftSize;
        fftBuf_[i] = inputRing_[src] * window_[i];
    }
    std::fill(fftBuf_ + kFftSize, fftBuf_ + kFftSize * 2, 0.0f);

    fft_.performRealOnlyForwardTransform(fftBuf_);

    float bins[kNumBins];
    for (int k = 0; k < kNumBins; ++k) {
        float re  = fftBuf_[2 * k];
        float im  = fftBuf_[2 * k + 1];
        float mag = std::sqrt(re * re + im * im);
        float db  = 20.0f * std::log10(std::max(mag, 1e-6f));
        bins[k] = std::clamp((db - kMinDb) / (kMaxDb - kMinDb), 0.0f, 1.0f);
    }

    sink_.pushFrame(bins);
}
