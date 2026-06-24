#version 460 core

out vec4 color;

in vec4 texel;


const int MAX_STOPS = 5;

uniform float colorStopPositions[MAX_STOPS];
uniform vec3  colorStopColors[MAX_STOPS];

vec4 heatmap(float normalizedValue) {
    normalizedValue = clamp(normalizedValue, 0.0, 1.0);

    int stopIndex = 0;
    while (stopIndex < MAX_STOPS - 2 && normalizedValue > colorStopPositions[stopIndex + 1]) {
        ++stopIndex;
    }

    float interpolationT = (normalizedValue - colorStopPositions[stopIndex]) / (colorStopPositions[stopIndex + 1] - colorStopPositions[stopIndex]);

    vec3 interpolatedColor = mix(colorStopColors[stopIndex], colorStopColors[stopIndex + 1], interpolationT);

    float alpha = mix(0, 1, int(normalizedValue>0.0001));

    return vec4(interpolatedColor, alpha);
}

void main() {
    // vec3 grayColor = vec3(texel.r);
    // color = vec4(grayColor, 1.0);

    color = heatmap(texel.r);
}
