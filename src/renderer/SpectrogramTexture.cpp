#include "renderer/SpectrogramTexture.h"

#include <algorithm>
#include <cstdint>

SpectrogramTexture::~SpectrogramTexture() {
    if (textureId != 0)
        glDeleteTextures(1, &textureId);
}

void SpectrogramTexture::create(int frequencyBinCount) {
    textureWidth = frequencyBinCount;

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, textureWidth, textureHeight, 0, GL_RED, GL_FLOAT, nullptr);

    glClearTexImage(textureId, 0, GL_RED, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void SpectrogramTexture::update(const SpectrogramBuffer& spectrogramBuffer) {
    if (textureId == 0) {
        create(spectrogramBuffer.getNumFrequencyBins());
    }

    const uint64_t writeIndex = spectrogramBuffer.getWriteIndex();
    const int newCursor = static_cast<int>(writeIndex % SpectrogramBuffer::maxFrames);
    const int validFrames = static_cast<int>(std::min<uint64_t>(writeIndex, SpectrogramBuffer::maxFrames));

    if (validFrames <= 0) {
        // Buffer emptied (e.g. eject): wipe the GPU texture so the mesh flattens.
        if (validFrameCount != 0) {
            glBindTexture(GL_TEXTURE_2D, textureId);
            glClearTexImage(textureId, 0, GL_RED, GL_FLOAT, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);
            lastUploadCursor = 0;
        }
        writeCursor = 0;
        validFrameCount = 0;
        return;
    }

    glBindTexture(GL_TEXTURE_2D, textureId);

    if (validFrameCount == 0) {
        const int firstSlot = (validFrames < SpectrogramBuffer::maxFrames) ? 0 : newCursor;
        for (int i = 0; i < validFrames; ++i) {
            const int slot = (firstSlot + i) % SpectrogramBuffer::maxFrames;
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, slot, textureWidth, 1,
                            GL_RED, GL_FLOAT, spectrogramBuffer.getSlotData(slot));
        }
    } else {
        for (int slot = lastUploadCursor; slot != newCursor; slot = (slot + 1) % SpectrogramBuffer::maxFrames) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, slot, textureWidth, 1,
                            GL_RED, GL_FLOAT, spectrogramBuffer.getSlotData(slot));
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    writeCursor = newCursor;
    validFrameCount = validFrames;
    lastUploadCursor = newCursor;
}

void SpectrogramTexture::bind(GLuint textureUnit) const {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureId);
}