#version 460 core

layout(location=0) in vec3 position;
layout(location=2) in vec2 uv;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 interp_uv;

void main() {

    interp_uv = uv;
    float scale = 0.7;
    gl_Position =  projectionMatrix * viewMatrix * modelMatrix * vec4(position * scale, 1.0f);
}
