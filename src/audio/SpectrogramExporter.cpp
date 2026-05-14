#include "../../include/audio/SpectrogramExporter.h"
#include "../../include/audio/STFTProcessor.h"

#include <juce_audio_formats/juce_audio_formats.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

namespace {

void heatmap(float normalizedValue,
             unsigned char& outRed,
             unsigned char& outGreen,
             unsigned char& outBlue)
{
    normalizedValue = std::clamp(normalizedValue, 0.0f, 1.0f);

    // 5-stop colormap: black(0) → blue(.25) → cyan(.5) → yellow(.75) → red(1)
    struct ColorStop { float position, red, green, blue; };
    static constexpr ColorStop colorStops[] = {
        { 0.00f, 0, 0, 0 },
        { 0.25f, 0, 0, 1 },
        { 0.50f, 0, 1, 1 },
        { 0.75f, 1, 1, 0 },
        { 1.00f, 1, 0, 0 },
    };
    constexpr int stopCount = int(sizeof(colorStops) / sizeof(colorStops[0]));

    int stopIndex = 0;
    while (stopIndex < stopCount - 2 && normalizedValue > colorStops[stopIndex + 1].position)
        ++stopIndex;

    const float interpolationT =
        (normalizedValue - colorStops[stopIndex].position) /
        (colorStops[stopIndex + 1].position - colorStops[stopIndex].position);

    outRed   = static_cast<unsigned char>(
        (colorStops[stopIndex].red   + interpolationT *
         (colorStops[stopIndex + 1].red   - colorStops[stopIndex].red))   * 255.f);
    outGreen = static_cast<unsigned char>(
        (colorStops[stopIndex].green + interpolationT *
         (colorStops[stopIndex + 1].green - colorStops[stopIndex].green)) * 255.f);
    outBlue  = static_cast<unsigned char>(
        (colorStops[stopIndex].blue  + interpolationT *
         (colorStops[stopIndex + 1].blue  - colorStops[stopIndex].blue))  * 255.f);
}

// Scrive un PPM dove pixelValueAccessor(column, row) ritorna il valore [0,1] del pixel.
template<typename PixelValueAccessor>
void writePPM(const std::string& outputPath,
              int imageWidth,
              int imageHeight,
              PixelValueAccessor pixelValueAccessor)
{
    std::ofstream outputStream(outputPath, std::ios::binary);
    if (!outputStream) {
        std::cerr << "[SpectrogramExporter] Cannot open: " << outputPath << "\n";
        return;
    }
    outputStream << "P6\n" << imageWidth << " " << imageHeight << "\n255\n";
    for (int row = imageHeight - 1; row >= 0; --row) { // basse frequenze in basso
        for (int column = 0; column < imageWidth; ++column) {
            unsigned char red, green, blue;
            heatmap(pixelValueAccessor(column, row), red, green, blue);
            outputStream.put(char(red)).put(char(green)).put(char(blue));
        }
    }
    std::cout << "[SpectrogramExporter] " << outputPath
              << "  (" << imageWidth << " frames x " << imageHeight << " bins)\n";
}

// Sink per STFTProcessor che accumula ogni frame in un vector.
struct FrameAccumulator {
    static constexpr int numFrequencyBins = 512;
    std::vector<std::array<float, numFrequencyBins>> accumulatedFrames;

    void pushFrame(const float* magnitudeBins) {
        accumulatedFrames.emplace_back();
        std::copy(magnitudeBins,
                  magnitudeBins + numFrequencyBins,
                  accumulatedFrames.back().data());
    }
};

} // namespace

void SpectrogramExporter::exportPPM(const SpectrogramBuffer& spectrogramBuffer,
                                    const std::string& outputPath)
{
    // Allochiamo per il caso peggiore (buffer pieno).
    // getSnapshot ci dirà quanti ne ha effettivamente copiati.
    std::vector<std::array<float, SpectrogramBuffer::numFrequencyBins>>
        snapshotFrames(SpectrogramBuffer::maxFrames);

    const int copiedFrameCount = spectrogramBuffer.getSnapshot(
        snapshotFrames.data(),
        SpectrogramBuffer::maxFrames);

    if (copiedFrameCount == 0) {
        std::cerr << "[SpectrogramExporter] No frames to export.\n";
        return;
    }

    writePPM(outputPath,
             copiedFrameCount,
             SpectrogramBuffer::numFrequencyBins,
             [&](int column, int row) { return snapshotFrames[column][row]; });
}

void SpectrogramExporter::exportFullTrack(const std::string& audioPath,
                                          const std::string& outputPath)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    formatManager.registerFormat(new juce::MP3AudioFormat(), true);

    std::unique_ptr<juce::AudioFormatReader> audioReader(
        formatManager.createReaderFor(juce::File(audioPath)));
    if (!audioReader) {
        std::cerr << "[SpectrogramExporter] Cannot read: " << audioPath << "\n";
        return;
    }

    FrameAccumulator frameAccumulator;
    STFTProcessor<FrameAccumulator> offlineStftProcessor(frameAccumulator);

    constexpr int chunkSampleCount = 4096;
    juce::AudioBuffer<float> audioChunk(
        static_cast<int>(audioReader->numChannels),
        chunkSampleCount);
    const int numChannels = audioChunk.getNumChannels();
    const float channelScale = numChannels > 0 ? 1.0f / static_cast<float>(numChannels) : 1.0f;
    std::vector<float> monoChunk(chunkSampleCount);

    juce::int64 currentReadPosition = 0;
    const juce::int64 totalSampleCount =
        static_cast<juce::int64>(audioReader->lengthInSamples);

    while (currentReadPosition < totalSampleCount) {
        const int samplesToRead = static_cast<int>(
            std::min<juce::int64>(chunkSampleCount,
                                  totalSampleCount - currentReadPosition));
        audioReader->read(&audioChunk, 0, samplesToRead, currentReadPosition, true, true);

        std::fill_n(monoChunk.begin(), samplesToRead, 0.0f);
        for (int channel = 0; channel < numChannels; ++channel) {
            const float* channelData = audioChunk.getReadPointer(channel);
            for (int sample = 0; sample < samplesToRead; ++sample)
                monoChunk[sample] += channelData[sample] * channelScale;
        }
        offlineStftProcessor.pushSamples(monoChunk.data(), samplesToRead);
        currentReadPosition += samplesToRead;
    }

    if (frameAccumulator.accumulatedFrames.empty()) {
        std::cerr << "[SpectrogramExporter] No frames produced.\n";
        return;
    }

    const int frameCount = static_cast<int>(frameAccumulator.accumulatedFrames.size());
    constexpr int binCount = FrameAccumulator::numFrequencyBins;
    writePPM(outputPath, frameCount, binCount,
             [&](int column, int row) {
                 return frameAccumulator.accumulatedFrames[column][row];
             });
}