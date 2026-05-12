#pragma once

#include <algorithm>
#include <mutex>

class SpectrogramBuffer {
public:
    static constexpr int maxFrames = 512;
    static constexpr int numFrequencyBins   = 512; // FFT_SIZE / 2

    void pushFrame(const float* magnitudeBins) {
        std::lock_guard<std::mutex> lock(bufferMutex);
        std::copy(magnitudeBins, magnitudeBins + numFrequencyBins, frameData[writeHead]);
        writeHead = (writeHead + 1) % maxFrames;
        if (storedFrameCount < maxFrames)
            ++storedFrameCount;
    }

    int getSnapshot(float (*destination)[numFrequencyBins], int requestedFrameCount) const {
        std::lock_guard<std::mutex> lock(bufferMutex);
        const int framesToCopy = std::min(storedFrameCount, requestedFrameCount);
        const int startIndex = (writeHead - framesToCopy + maxFrames) % maxFrames;
        for (int i = 0; i < framesToCopy; ++i) {
            int sourceIndex = (startIndex + i) % maxFrames;
            std::copy(frameData[sourceIndex], frameData[sourceIndex] + numFrequencyBins, destination[i]);
        }

        return framesToCopy;
    }

    int totalFrames() const {
        std::lock_guard<std::mutex> lock(bufferMutex);
        return storedFrameCount;
    }

private:
    mutable std::mutex bufferMutex;
    float frameData[maxFrames][numFrequencyBins]{};
    int   writeHead   = 0;
    int   storedFrameCount = 0;
};
