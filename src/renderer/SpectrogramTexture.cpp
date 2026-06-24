#include "../../include/renderer/SpectrogramTexture.h"

SpectrogramTexture::~SpectrogramTexture() {
    if (textureId != 0)
        glDeleteTextures(1, &textureId);
}

void SpectrogramTexture::init() {
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
        return;
    }

    const uint64_t snap = spectrogramBuffer.getWriteIndex();
    const int newCursor = static_cast<int>(snap % SpectrogramBuffer::maxFrames);
    const int valid = static_cast<int>(std::min(snap, static_cast<uint64_t>(SpectrogramBuffer::maxFrames)));

    if (valid <= 0) {
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
        const int start = (valid < SpectrogramBuffer::maxFrames) ? 0 : newCursor;
        for (int i = 0; i < valid; ++i) {
            const int slot = (start + i) % SpectrogramBuffer::maxFrames;
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, slot, textureWidth, 1,
                            GL_RED, GL_FLOAT, spectrogramBuffer.getSlotData(slot));
        }
    } else {
        int cursor = lastUploadCursor;
        while (cursor != newCursor) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, cursor, textureWidth, 1,
                            GL_RED, GL_FLOAT, spectrogramBuffer.getSlotData(cursor));
            cursor = (cursor + 1) % SpectrogramBuffer::maxFrames;
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    writeCursor = newCursor;
    validFrameCount = valid;
    lastUploadCursor = newCursor;
}

void SpectrogramTexture::bind(GLuint textureUnit) const {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureId);
}
