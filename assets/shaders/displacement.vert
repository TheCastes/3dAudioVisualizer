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

out float magnitude;

void main() {
    float binCount = float(textureSize(spectrogram, 0).x);
    float ringSize = float(textureSize(spectrogram, 0).y);
    float baseFreq = uv.y * binCount;
    float maxAge = max(float(validFrames - 1 - temporalWindow), 0.0);
    float baseAge = (1.0 - uv.x) * maxAge;

    int halfFreq = freqSampleSize / 2;
    float weightedSum = 0.0;
    float weightSum = 0.0;
    for (int timeStep = 0; timeStep < temporalWindow; ++timeStep) {
        float weight = exp(-float(timeStep * timeStep) / (2.0 * temporalSigma * temporalSigma));
        float row = mod(float(writeCursor) - 1.0 - (baseAge + float(timeStep)) + ringSize, ringSize);
        float timeCoord = (row + 0.5) / ringSize;
        for (int freqStep = -halfFreq; freqStep < freqSampleSize - halfFreq; ++freqStep) {
            float frequencyBin = clamp(baseFreq + float(freqStep), 0.0, binCount - 1.0);
            float freqCoord = (frequencyBin + 0.5) / binCount;
            weightedSum += texture(spectrogram, vec2(freqCoord, timeCoord)).r * weight;
            weightSum += weight;
        }
    }
    magnitude = weightedSum / weightSum;

    vec3 displacement = vec3(0.0, 0.0, magnitude);

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position + displacement, 1.0);
}