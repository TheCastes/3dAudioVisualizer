#pragma once

#include <glad/glad.h>

#include <array>
#include <vector>

#include "../audio/SpectrogramBuffer.h"

class SpectrogramTexture {
public:
    SpectrogramTexture() = default;
    ~SpectrogramTexture();

    SpectrogramTexture(const SpectrogramTexture& copy) = delete;
    SpectrogramTexture& operator=(const SpectrogramTexture&) = delete;
    SpectrogramTexture(SpectrogramTexture&& move) = delete;
    SpectrogramTexture& operator=(SpectrogramTexture&&) = delete;

    void init();

    void update(const SpectrogramBuffer& spectrogramBuffer);

    void bind(GLuint textureUnit = 0) const;

    GLuint id() const { return textureId; }

    int validFrames() const { return uploadedFrameCount; }

private:
    static constexpr int textureWidth  = SpectrogramBuffer::numFrequencyBins;
    static constexpr int textureHeight = SpectrogramBuffer::maxFrames;

    GLuint textureId = 0;
    int uploadedFrameCount = 0;

    std::vector<std::array<float, SpectrogramBuffer::numFrequencyBins>> stagingFrames;
};