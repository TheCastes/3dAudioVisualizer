#pragma once

#include <glad/glad.h>

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

    int getWriteCursor() const { return writeCursor; }
    int getValidFrameCount() const { return validFrameCount; }

private:
    void create(int frequencyBinCount);

    static constexpr int textureHeight = SpectrogramBuffer::maxFrames;

    int textureWidth = 0;
    GLuint textureId = 0;

    int writeCursor = 0;
    int validFrameCount = 0;
    int lastUploadCursor = 0;
};