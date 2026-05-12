#include "../../include/audio/SpectrogramExporter.h"
#include "../../include/audio/STFTProcessor.h"

#include <juce_audio_formats/juce_audio_formats.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

// ---------------------------------------------------------------------------
// Local helpers
// ---------------------------------------------------------------------------

namespace {

void heatmap(float v, unsigned char& r, unsigned char& g, unsigned char& b) {
    v = std::clamp(v, 0.0f, 1.0f);

    // 5-stop colormap: black(0) → blue(.25) → cyan(.5) → yellow(.75) → red(1)
    struct Stop { float pos, r, g, b; };
    static constexpr Stop stops[] = {
        { 0.00f, 0, 0, 0 },
        { 0.25f, 0, 0, 1 },
        { 0.50f, 0, 1, 1 },
        { 0.75f, 1, 1, 0 },
        { 1.00f, 1, 0, 0 },
    };
    constexpr int nStops = int(sizeof(stops) / sizeof(stops[0]));

    int i = 0;
    while (i < nStops - 2 && v > stops[i + 1].pos)
        ++i;

    const float t = (v - stops[i].pos) / (stops[i + 1].pos - stops[i].pos);
    r = static_cast<unsigned char>((stops[i].r + t * (stops[i + 1].r - stops[i].r)) * 255.f);
    g = static_cast<unsigned char>((stops[i].g + t * (stops[i + 1].g - stops[i].g)) * 255.f);
    b = static_cast<unsigned char>((stops[i].b + t * (stops[i + 1].b - stops[i].b)) * 255.f);
}

// Writes a PPM where getValue(col, row) returns the [0,1] value for that pixel.
template<typename F>
void writePPM(const std::string& path, int W, int H, F getValue) {
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        std::cerr << "[SpectrogramExporter] Cannot open: " << path << "\n";
        return;
    }
    ofs << "P6\n" << W << " " << H << "\n255\n";
    for (int row = H - 1; row >= 0; --row) { // low frequency at bottom
        for (int col = 0; col < W; ++col) {
            unsigned char r, g, b;
            heatmap(getValue(col, row), r, g, b);
            ofs.put(char(r)).put(char(g)).put(char(b));
        }
    }
    std::cout << "[SpectrogramExporter] " << path
              << "  (" << W << " frames x " << H << " bins)\n";
}

// Sink for STFTProcessor that accumulates every frame into a vector.
struct FrameAccumulator {
    static constexpr int numFrequencyBins = 512; // matches STFTProcessor::numFrequencyBins
    std::vector<std::array<float, numFrequencyBins>> frames;

    void pushFrame(const float* bins) {
        frames.emplace_back();
        std::copy(bins, bins + numFrequencyBins, frames.back().data());
    }
};

} // namespace

// ---------------------------------------------------------------------------
// SpectrogramExporter
// ---------------------------------------------------------------------------

void SpectrogramExporter::exportPPM(const SpectrogramBuffer& buf, const std::string& path) {
    const int numFrames = buf.totalFrames();
    if (numFrames == 0) {
        std::cerr << "[SpectrogramExporter] No frames to export.\n";
        return;
    }

    const int n = std::min(numFrames, SpectrogramBuffer::maxFrames);
    std::vector<std::array<float, SpectrogramBuffer::numFrequencyBins> > snapshot(n);
    buf.getSnapshot(
        reinterpret_cast<float(*)[SpectrogramBuffer::numFrequencyBins]>(snapshot.data()), n);

    writePPM(path, n, SpectrogramBuffer::numFrequencyBins,
             [&](int col, int row) { return snapshot[col][row]; });
}

void SpectrogramExporter::exportFullTrack(const std::string& audioPath, const std::string& outPath) {
    juce::AudioFormatManager fmt;
    fmt.registerBasicFormats();
    fmt.registerFormat(new juce::MP3AudioFormat(), true);

    std::unique_ptr<juce::AudioFormatReader> reader(
        fmt.createReaderFor(juce::File(audioPath)));
    if (!reader) {
        std::cerr << "[SpectrogramExporter] Cannot read: " << audioPath << "\n";
        return;
    }

    FrameAccumulator acc;
    STFTProcessor<FrameAccumulator> stft(acc);

    constexpr int chunkSize = 4096;
    juce::AudioBuffer<float> chunk(static_cast<int>(reader->numChannels), chunkSize);
    const int numCh       = chunk.getNumChannels();
    const float scale     = numCh > 0 ? 1.0f / static_cast<float>(numCh) : 1.0f;
    std::vector<float> mono(chunkSize);

    juce::int64 pos = 0;
    const juce::int64 total = static_cast<juce::int64>(reader->lengthInSamples);

    while (pos < total) {
        const int toRead = static_cast<int>(std::min<juce::int64>(chunkSize, total - pos));
        reader->read(&chunk, 0, toRead, pos, true, true);

        std::fill_n(mono.begin(), toRead, 0.0f);
        for (int ch = 0; ch < numCh; ++ch) {
            const float* src = chunk.getReadPointer(ch);
            for (int i = 0; i < toRead; ++i)
                mono[i] += src[i] * scale;
        }
        stft.pushSamples(mono.data(), toRead);
        pos += toRead;
    }

    if (acc.frames.empty()) {
        std::cerr << "[SpectrogramExporter] No frames produced.\n";
        return;
    }

    const int W = static_cast<int>(acc.frames.size());
    constexpr int H = FrameAccumulator::numFrequencyBins;
    writePPM(outPath, W, H,
             [&](int col, int row) { return acc.frames[col][row]; });
}
