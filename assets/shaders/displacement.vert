#version 460 core

layout(location=0) in vec3 position;
layout(location=2) in vec2 uv;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform sampler2D spectrogram;
uniform int writeCursor;
uniform int validFrames;
uniform int temporalWindow;
uniform float temporalSigma;
uniform int freqSampleSize;

out vec4 texel;

void main() {
    float binCount = float(textureSize(spectrogram, 0).x);
    float ringSize = float(textureSize(spectrogram, 0).y);
    float baseFreq = uv.y * binCount;
    float maxAge = max(float(validFrames - 1 - temporalWindow), 0.0);
    float baseAge = (1.0 - uv.x) * maxAge;

    int halfFreq = freqSampleSize / 2;
    float weightedSum = 0.0;
    float weightSum = 0.0;
    for (int ti = 0; ti < temporalWindow; ++ti) {
        float wt = exp(-float(ti * ti) / (2.0 * temporalSigma * temporalSigma));
        float row = mod(float(writeCursor) - 1.0 - (baseAge + float(ti)) + ringSize, ringSize);
        float t_coord = (row + 0.5) / ringSize;
        for (int fi = -halfFreq; fi < freqSampleSize - halfFreq; ++fi) {
            float fbin = clamp(baseFreq + float(fi), 0.0, binCount - 1.0);
            float freq = (fbin + 0.5) / binCount;
            weightedSum += texture(spectrogram, vec2(freq, t_coord)).r * wt;
            weightSum += wt;
        }
    }
    texel = vec4(weightedSum / weightSum, 0.0, 0.0, 1.0);

    float scale = 0.7;

    vec3 displacement = vec3(0,0,(texel.r));

    gl_Position =  projectionMatrix * viewMatrix * modelMatrix * vec4((position * scale)+(displacement), 1.0f);
}
