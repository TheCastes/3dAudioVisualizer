#pragma once

#include <array>
#include <atomic>
#include <algorithm>
#include <cstdint>

class SpectrogramBuffer {
public:
    static constexpr int maxFrames = 512;
    static constexpr int numFrequencyBins = 512; // FFT_SIZE / 2

    void pushFrame(const float* magnitudeBins) {
        const uint64_t writeIndex = totalFramesWritten.load(std::memory_order_relaxed);
        const int slot = static_cast<int>(writeIndex % maxFrames);

        std::copy(magnitudeBins, magnitudeBins + numFrequencyBins, frameStorage[slot].data());

        totalFramesWritten.store(writeIndex + 1, std::memory_order_release);
    }

    int getSnapshot(std::array<float, numFrequencyBins>* destinationFrames, int requestedFrameCount) const {
        const uint64_t writeIndexSnapshot = totalFramesWritten.load(std::memory_order_acquire);
        const int framesAvailable = static_cast<int>(std::min<uint64_t>(writeIndexSnapshot, maxFrames));
        const int framesToCopy = std::min(framesAvailable, requestedFrameCount);

        if (framesToCopy <= 0) {
            return 0;
        }
        const uint64_t firstFrameAbsoluteIndex = writeIndexSnapshot - framesToCopy;

        for (int i = 0; i < framesToCopy; ++i) {
            const int sourceSlot = static_cast<int>((firstFrameAbsoluteIndex + i) % maxFrames);
            std::copy(frameStorage[sourceSlot].begin(), frameStorage[sourceSlot].end(), destinationFrames[i].begin());
        }

        return framesToCopy;
    }

    void clear() {
        totalFramesWritten.store(0, std::memory_order_release);
    }

    int totalFrames() const {
        const uint64_t writeIndexSnapshot = totalFramesWritten.load(std::memory_order_acquire);
        return static_cast<int>(std::min<uint64_t>(writeIndexSnapshot, maxFrames));
    }

private:
    std::array<std::array<float, numFrequencyBins>, maxFrames> frameStorage{};
    std::atomic<uint64_t> totalFramesWritten{0};
};
