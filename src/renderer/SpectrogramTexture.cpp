#include "../../include/renderer/SpectrogramTexture.h"

#include <cstddef>

SpectrogramTexture::~SpectrogramTexture() {
    if (textureId != 0)
        glDeleteTextures(1, &textureId);
}

void SpectrogramTexture::create(int frequencyBinCount) {
    textureWidth = frequencyBinCount;
    stagingFrames.resize(static_cast<std::size_t>(textureHeight) * frequencyBinCount);

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, textureWidth, textureHeight, 0, GL_RED, GL_FLOAT, nullptr);

    glClearTexImage(textureId, 0, GL_RED, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void SpectrogramTexture::update(const SpectrogramBuffer& spectrogramBuffer) {
    if (textureId == 0) {
        create(spectrogramBuffer.numFrequencyBins());
    }

    const int frameCount = spectrogramBuffer.getSnapshot(stagingFrames.data(), textureHeight);

    if (frameCount <= 0) {
        // Buffer emptied (e.g. eject): wipe the GPU texture so the mesh flattens.
        if (uploadedFrameCount != 0) {
            glBindTexture(GL_TEXTURE_2D, textureId);
            glClearTexImage(textureId, 0, GL_RED, GL_FLOAT, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);
            uploadedFrameCount = 0;
        }
        return;
    }

    uploadedFrameCount = frameCount;

    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, uploadedFrameCount, GL_RED, GL_FLOAT, stagingFrames.data());

    glBindTexture(GL_TEXTURE_2D, 0);
}

void SpectrogramTexture::bind(GLuint textureUnit) const {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureId);
}