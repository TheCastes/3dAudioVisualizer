#include "../../include/renderer/SpectrogramTexture.h"

SpectrogramTexture::~SpectrogramTexture() {
    if (textureId != 0)
        glDeleteTextures(1, &textureId);
}

void SpectrogramTexture::init() {
    stagingFrames.resize(SpectrogramBuffer::maxFrames);

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
        return;
    }

    uploadedFrameCount = spectrogramBuffer.getSnapshot(stagingFrames.data(), SpectrogramBuffer::maxFrames);
    
    if (uploadedFrameCount <= 0) {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, uploadedFrameCount, GL_RED, GL_FLOAT, stagingFrames.data());

    glBindTexture(GL_TEXTURE_2D, 0);
}

void SpectrogramTexture::bind(GLuint textureUnit) const {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureId);
}