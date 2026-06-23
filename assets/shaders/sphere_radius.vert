#version 460 core

layout(location=0) in vec2 corner;
layout(location=1) in vec3 center;
layout(location=2) in vec2 uv;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform sampler2D spectrogram;
uniform float radiusScale;

out vec2 localPos;
flat out vec3 centerView;
flat out float radiusView;
flat out float mag;

void main() {
    mag = texture(spectrogram, vec2(uv.y, uv.x)).r;

    vec3 worldCenter = center;
    float r = mag * radiusScale;

    centerView = (viewMatrix * modelMatrix * vec4(worldCenter, 1.0)).xyz;
    radiusView = r;

    localPos = corner * 2.0;
    vec3 cornerView = centerView + vec3(localPos * r, 0.0);
    gl_Position = projectionMatrix * vec4(cornerView, 1.0);
}
