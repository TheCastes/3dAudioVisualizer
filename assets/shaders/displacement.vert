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

out vec4 texel;

void main() {
    float age = (1.0 - uv.x) * float(validFrames - 1);

    float weightedSum = 0.0;
    float weightSum = 0.0;
    for (int ti = 0; ti < temporalWindow; ++ti) {
        float a = age + float(ti);
        float row_f = mod(float(writeCursor) - 1.0 - a + 512.0, 512.0);
        float t_coord = (row_f + 0.5) / 512.0;
        float w = exp(-float(ti * ti) / (2.0 * temporalSigma * temporalSigma));
        weightedSum += texture(spectrogram, vec2(uv.y, t_coord)).r * w;
        weightSum += w;
    }
    texel = vec4(weightedSum / weightSum, 0.0, 0.0, 1.0);

    float scale = 0.7;

    vec3 displacement = vec3(0,0,(texel.r));

    gl_Position =  projectionMatrix * viewMatrix * modelMatrix * vec4((position * scale)+(displacement), 1.0f);
}
