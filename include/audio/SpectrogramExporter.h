#pragma once

#include <string>
#include "SpectrogramBuffer.h"

class SpectrogramExporter {
public:
    static void exportPPM(const SpectrogramBuffer& buf, const std::string& path);

private:
    static void heatmap(float v, unsigned char& r, unsigned char& g, unsigned char& b);
};
