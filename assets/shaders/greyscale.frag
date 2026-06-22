#version 460 core

out vec4 color;

in vec2 interp_uv;
in vec4 texel;



void main() {
    vec3 grayColor = vec3(texel.r);

    float normalizedValue = clamp(texel.r, 0.0, 1.0); // ?
    float alpha = mix(0, 1, int(normalizedValue>0.0001));

    color = vec4(grayColor, alpha);
}
