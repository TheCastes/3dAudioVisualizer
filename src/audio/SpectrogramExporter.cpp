#include "../../include/audio/SpectrogramExporter.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

void SpectrogramExporter::heatmap(float v, unsigned char& r, unsigned char& g, unsigned char& b) {
    v = std::clamp(v, 0.0f, 1.0f);

    // 5-stop colormap: black(0) → blue(.25) → cyan(.5) → yellow(.75) → red(1)
    struct Stop { float pos; float r, g, b; };
    static constexpr Stop stops[] = {
        { 0.00f,  0, 0, 0 },
        { 0.25f,  0, 0, 1 },
        { 0.50f,  0, 1, 1 },
        { 0.75f,  1, 1, 0 },
        { 1.00f,  1, 0, 0 },
    };
    constexpr int nStops = int(sizeof(stops) / sizeof(stops[0]));

    int i = 0;
    while (i < nStops - 2 && v > stops[i + 1].pos)
        ++i;

    float t = (v - stops[i].pos) / (stops[i + 1].pos - stops[i].pos);
    r = (unsigned char)((stops[i].r + t * (stops[i + 1].r - stops[i].r)) * 255.f);
    g = (unsigned char)((stops[i].g + t * (stops[i + 1].g - stops[i].g)) * 255.f);
    b = (unsigned char)((stops[i].b + t * (stops[i + 1].b - stops[i].b)) * 255.f);
}

void SpectrogramExporter::exportPPM(const SpectrogramBuffer& buf, const std::string& path) {
    static float snapshot[SpectrogramBuffer::kMaxFrames][SpectrogramBuffer::kNumBins];
    const int frames = buf.getSnapshot(snapshot, SpectrogramBuffer::kMaxFrames);

    if (frames == 0) {
        std::cerr << "[SpectrogramExporter] No frames to export.\n";
        return;
    }

    const int W = frames; // time axis
    const int H = SpectrogramBuffer::kNumBins; // frequency axis

    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        std::cerr << "[SpectrogramExporter] Cannot open: " << path << "\n";
        return;
    }

    ofs << "P6\n" << W << " " << H << "\n255\n";

    for (int row = H - 1; row >= 0; --row) { // flip: low frequency at bottom
        for (int col = 0; col < W; ++col) {
            unsigned char r, g, b;
            heatmap(snapshot[col][row], r, g, b);
            ofs.put(char(r)).put(char(g)).put(char(b));
        }
    }

    std::cout << "[SpectrogramExporter] " << path
              << "  (" << W << " frames × " << H << " bins)\n";
}
