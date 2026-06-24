#version 460 core

layout(location=0) in vec2 corner;
layout(location=1) in vec3 center;
layout(location=2) in vec2 uv;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform sampler2D spectrogram;
uniform float radiusScale;
uniform int writeCursor;
uniform int validFrames;
uniform int sampleSize;

out vec2 localPos;
flat out vec3 centerView;
flat out float radiusView;
flat out float mag;

void main() {
    float baseFreq = uv.y * 512.0;
    float baseAge = (1.0 - uv.x) * float(validFrames - 1);

    int halfSize = sampleSize / 2;
    float sum = 0.0;
    int total = 0;
    for (int fi = -halfSize; fi < sampleSize - halfSize; ++fi) {
        for (int ti = -halfSize; ti < sampleSize - halfSize; ++ti) {
            float freq = (baseFreq + float(fi)) / 512.0;
            float age = baseAge + float(ti);
            float row_f = mod(float(writeCursor) - 1.0 - age + 512.0, 512.0);
            float t_coord = (row_f + 0.5) / 512.0;
            sum += texture(spectrogram, vec2(freq, t_coord)).r;
            total++;
        }
    }
    mag = sum / float(total);

    vec3 worldCenter = center;
    float r = mag * radiusScale;

    centerView = (viewMatrix * modelMatrix * vec4(worldCenter, 1.0)).xyz;
    radiusView = r;

    localPos = corner * 2.0;
    vec3 cornerView = centerView + vec3(localPos * r, 0.0);
    gl_Position = projectionMatrix * vec4(cornerView, 1.0);
}
