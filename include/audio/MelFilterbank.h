#pragma once

#include <cmath>
#include <cstddef>
#include <vector>

class MelFilterbank {
public:
    MelFilterbank() = default;

    void prepare(double sampleRate, int fftSize, int frequencyBinCount, int melBandCount) {
        this->frequencyBinCount = frequencyBinCount;
        this->melBandCount = melBandCount;
        filterWeights.assign(static_cast<std::size_t>(melBandCount) * frequencyBinCount, 0.0f);

        const float lowestMel  = hertzToMel(0.0f);
        const float highestMel = hertzToMel(static_cast<float>(sampleRate) * 0.5f);

        std::vector<float> bandEdgeBins(melBandCount + 2);
        for (int edgeIndex = 0; edgeIndex < melBandCount + 2; ++edgeIndex) {
            const float mel = lowestMel + (highestMel - lowestMel) * edgeIndex / (melBandCount + 1);
            const float hertz = melToHertz(mel);
            bandEdgeBins[edgeIndex] = hertz * fftSize / static_cast<float>(sampleRate);
        }

        for (int band = 0; band < melBandCount; ++band) {
            const float leftBin = bandEdgeBins[band];
            const float centerBin = bandEdgeBins[band + 1];
            const float rightBin = bandEdgeBins[band + 2];
            float* bandWeights = &filterWeights[static_cast<std::size_t>(band) * frequencyBinCount];

            float weightSum = 0.0f;
            for (int bin = 0; bin < frequencyBinCount; ++bin) {
                float weight = 0.0f;
                if (bin >= leftBin && bin <= centerBin && centerBin > leftBin)
                    weight = (bin - leftBin) / (centerBin - leftBin);
                else if (bin > centerBin && bin <= rightBin && rightBin > centerBin)
                    weight = (rightBin - bin) / (rightBin - centerBin);
                bandWeights[bin] = weight;
                weightSum += weight;
            }

            if (weightSum > 0.0f)
                for (int bin = 0; bin < frequencyBinCount; ++bin)
                    bandWeights[bin] /= weightSum;
        }
    }

    void apply(const float* linearMagnitude, float* melBandsOut) const {
        for (int band = 0; band < melBandCount; ++band) {
            const float* bandWeights = &filterWeights[static_cast<std::size_t>(band) * frequencyBinCount];
            float bandEnergy = 0.0f;
            for (int bin = 0; bin < frequencyBinCount; ++bin)
                bandEnergy += bandWeights[bin] * linearMagnitude[bin];
            melBandsOut[band] = bandEnergy;
        }
    }

    int getMelBandCount() const { return melBandCount; }

private:
    static float hertzToMel(float hertz) { return 2595.0f * std::log10(1.0f + hertz / 700.0f); }
    static float melToHertz(float mel) { return 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f); }

    int frequencyBinCount = 0;
    int melBandCount = 0;
    std::vector<float> filterWeights;
};