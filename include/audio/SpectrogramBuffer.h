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
        : numFrequencyBins(numFrequencyBins),
          frameStorage(static_cast<std::size_t>(maxFrames) * numFrequencyBins, 0.0f) {}

    void pushFrame(const float* magnitudeBins) {
        const uint64_t writeIndex = totalFramesWritten.load(std::memory_order_relaxed);
        const int slot = static_cast<int>(writeIndex % maxFrames);

        std::copy(magnitudeBins, magnitudeBins + numFrequencyBins, &frameStorage[static_cast<std::size_t>(slot) * numFrequencyBins]);

        totalFramesWritten.store(writeIndex + 1, std::memory_order_release);
    }

    void clear() {
        totalFramesWritten.store(0, std::memory_order_release);
    }

    void preFill() {
        std::vector<float> zeros(numFrequencyBins, 0.0f);
        for (int i = 0; i < maxFrames; ++i)
            pushFrame(zeros.data());
    }

    const float* getSlotData(int slot) const {
        return &frameStorage[static_cast<std::size_t>(slot) * numFrequencyBins];
    }

    uint64_t getWriteIndex() const {
        return totalFramesWritten.load(std::memory_order_acquire);
    }

    int getNumFrequencyBins() const { return numFrequencyBins; }

private:
    int numFrequencyBins;
    std::vector<float> frameStorage;
    std::atomic<uint64_t> totalFramesWritten{0};
};