#pragma once

#include <algorithm>
#include <mutex>

class SpectrogramBuffer {
public:
    static constexpr int kMaxFrames = 512;
    static constexpr int kNumBins   = 512; // FFT_SIZE / 2

    void pushFrame(const float* bins) {
        std::lock_guard<std::mutex> lk(mtx_);
        std::copy(bins, bins + kNumBins, data_[writeHead_]);
        writeHead_ = (writeHead_ + 1) % kMaxFrames;
        if (totalFrames_ < kMaxFrames)
            ++totalFrames_;
    }

    int getSnapshot(float (*dst)[kNumBins], int maxFrames) const {
        std::lock_guard<std::mutex> lk(mtx_);
        int n     = std::min(totalFrames_, maxFrames);
        int start = (writeHead_ - n + kMaxFrames) % kMaxFrames;
        for (int i = 0; i < n; ++i) {
            int src = (start + i) % kMaxFrames;
            std::copy(data_[src], data_[src] + kNumBins, dst[i]);
        }
        return n;
    }

    int totalFrames() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return totalFrames_;
    }

private:
    mutable std::mutex mtx_;
    float data_[kMaxFrames][kNumBins]{};
    int   writeHead_   = 0;
    int   totalFrames_ = 0;
};
