#version 460 core

out vec4 color;

in vec2 localPos;
flat in vec3 centerView;
flat in float radiusView;
flat in float mag;

uniform mat4 projectionMatrix;

const int MAX_STOPS = 5;
uniform float colorStopPositions[MAX_STOPS];
uniform vec3  colorStopColors[MAX_STOPS];

vec3 colormap(float normalizedValue) {
    normalizedValue = clamp(normalizedValue, 0.0, 1.0);

    int stopIndex = 0;
    while (stopIndex < MAX_STOPS - 2 && normalizedValue > colorStopPositions[stopIndex + 1]) {
        ++stopIndex;
    }

    float interpolationT = (normalizedValue - colorStopPositions[stopIndex]) / (colorStopPositions[stopIndex + 1] - colorStopPositions[stopIndex]);

    return mix(colorStopColors[stopIndex], colorStopColors[stopIndex + 1], interpolationT);
}

void main() {
    float r2 = dot(localPos, localPos);
    if (r2 > 1.0 || mag <= 0.0001) discard;

    float z = sqrt(1.0 - r2);
    vec3 normal = vec3(localPos, z);
    vec3 viewPos = centerView + radiusView * normal;

    vec4 clip = projectionMatrix * vec4(viewPos, 1.0);
    gl_FragDepth = clip.z / clip.w * 0.5 + 0.5;

    const vec3 lightDir = normalize(vec3(-0.4, 0.6, 0.8));
    float lambert = 0.2 + 0.8 * max(dot(normal, lightDir), 0.0);

    color = vec4(colormap(mag) * lambert, 1.0);
}