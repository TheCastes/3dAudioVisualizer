#version 460 core

layout(location=0) in vec3 position;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
    float scale = 0.7;
    gl_Position =  projectionMatrix * viewMatrix * modelMatrix * vec4(position * scale, 1.0f);
}
