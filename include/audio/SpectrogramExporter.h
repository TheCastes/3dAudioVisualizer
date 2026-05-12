#pragma once

#include <string>
#include "SpectrogramBuffer.h"

class SpectrogramExporter {
public:
    // Export the last N frames from the real-time ring buffer.
    static void exportPPM(const SpectrogramBuffer& buf, const std::string& path);

    // Offline: decode the entire audio file, run STFT, export full spectrogram.
    static void exportFullTrack(const std::string& audioPath, const std::string& outPath);
};