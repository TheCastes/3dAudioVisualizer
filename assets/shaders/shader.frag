#version 460 core

out vec4 color;
uniform float u_level;
uniform sampler2D texSamp;

in vec2 interp_uv;

void main() {
    vec4 texel = texture(texSamp, interp_uv);
    vec3 grayColor = vec3(texel.r);
    color = vec4(grayColor, 1.0);
}