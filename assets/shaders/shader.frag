#version 460 core

out vec4 color;
uniform float u_level;

void main() {
    color = vec4(u_level, 0.3, 1.0 - u_level, 1.0);
}