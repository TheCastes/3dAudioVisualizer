#pragma once

#include <glad/glad.h>

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

    void update(const SpectrogramBuffer& spectrogramBuffer);

    void bind(GLuint textureUnit = 0) const;

    GLuint id() const { return textureId; }

    int validFrames() const { return uploadedFrameCount; }

private:
    void create(int frequencyBinCount);

    static constexpr int textureHeight = SpectrogramBuffer::maxFrames;

    int textureWidth = 0;
    GLuint textureId = 0;
    int uploadedFrameCount = 0;

    std::vector<float> stagingFrames;
};