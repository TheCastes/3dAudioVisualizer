#pragma once

#include <atomic>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

class SpectrogramBuffer {
public:
    static constexpr int maxFrames = 512;

    explicit SpectrogramBuffer(int numFrequencyBins)
        : numBins(numFrequencyBins),
          frameStorage(static_cast<std::size_t>(maxFrames) * numFrequencyBins, 0.0f) {}

    void pushFrame(const float* magnitudeBins) {
        const uint64_t writeIndex = totalFramesWritten.load(std::memory_order_relaxed);
        const int slot = static_cast<int>(writeIndex % maxFrames);

        std::copy(magnitudeBins, magnitudeBins + numBins,
                  &frameStorage[static_cast<std::size_t>(slot) * numBins]);

        totalFramesWritten.store(writeIndex + 1, std::memory_order_release);
    }

    int getSnapshot(float* destinationFrames, int requestedFrameCount) const {
        const uint64_t writeIndexSnapshot = totalFramesWritten.load(std::memory_order_acquire);
        const int framesAvailable = static_cast<int>(std::min<uint64_t>(writeIndexSnapshot, maxFrames));
        const int framesToCopy = std::min(framesAvailable, requestedFrameCount);

        if (framesToCopy <= 0) {
            return 0;
        }
        const uint64_t firstFrameAbsoluteIndex = writeIndexSnapshot - framesToCopy;

        for (int i = 0; i < framesToCopy; ++i) {
            const int sourceSlot = static_cast<int>((firstFrameAbsoluteIndex + i) % maxFrames);
            const float* sourceRow = &frameStorage[static_cast<std::size_t>(sourceSlot) * numBins];
            std::copy(sourceRow, sourceRow + numBins,
                      destinationFrames + static_cast<std::size_t>(i) * numBins);
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

    int numFrequencyBins() const { return numBins; }

private:
    int numBins;
    std::vector<float> frameStorage;
    std::atomic<uint64_t> totalFramesWritten{0};
};