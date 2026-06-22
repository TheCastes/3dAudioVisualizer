#version 460 core

out vec4 color;

in vec2 interp_uv;
in vec4 texel;


const struct ColorStop { float position; vec3 color; };
const int MAX_STOPS = 5;
const ColorStop colorStops[MAX_STOPS] = ColorStop[MAX_STOPS] (
    ColorStop( 0.00, vec3(0.0, 0.0, 0.0) ),
    ColorStop( 0.25, vec3(0.0, 0.0, 1.0) ),
    ColorStop( 0.50, vec3(0.0, 1.0, 1.0) ),
    ColorStop( 0.75, vec3(1.0, 1.0, 0.0) ),
    ColorStop( 1.00, vec3(1.0, 0.0, 0.0) )
);

vec4 heatmap(float normalizedValue) {
    normalizedValue = clamp(normalizedValue, 0.0, 1.0);

    int stopCount = colorStops.length();

    int stopIndex = 0;
    while (stopIndex < stopCount - 2 && normalizedValue > colorStops[stopIndex + 1].position) {
        ++stopIndex;
    }

    float interpolationT = (normalizedValue - colorStops[stopIndex].position) / (colorStops[stopIndex + 1].position - colorStops[stopIndex].position);

    vec3 interpolatedColor = mix( colorStops[stopIndex].color, colorStops[stopIndex + 1].color, interpolationT);

    float alpha = mix(0, 1, int(normalizedValue>0.0001));

    return vec4(interpolatedColor, alpha);
}

void main() {
    // vec3 grayColor = vec3(texel.r);
    // color = vec4(grayColor, 1.0);

    color = heatmap(texel.r);
}
