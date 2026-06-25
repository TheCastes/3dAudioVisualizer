#version 460 core

out vec4 color;

in float magnitude;


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

    vec3 interpolatedColor = mix(colorStopColors[stopIndex], colorStopColors[stopIndex + 1], interpolationT);

    return interpolatedColor;
}

void main() {
    color.rgb = colormap(magnitude);
    color.a = mix(0, 1, int(magnitude > 0.0001));
}