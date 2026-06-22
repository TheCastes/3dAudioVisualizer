#version 460 core

out vec4 color;

in vec2 interp_uv;
in vec4 texel;



void main() {
    vec3 grayColor = vec3(texel.r);
    color = vec4(grayColor, 1.0);
}
