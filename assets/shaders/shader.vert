#version 460 core

layout(location=0) in vec3 position;
layout(location=2) in vec2 uv;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform sampler2D texSamp;

out vec4 texel;
out vec2 interp_uv;

void main() {
    interp_uv = vec2(uv[1],uv[0]);
    texel = texture(texSamp, interp_uv);
    float scale = 0.7;

    vec3 displacement = vec3(0,0,texel.r);

    gl_Position =  projectionMatrix * viewMatrix * modelMatrix * vec4((position * scale)+(displacement), 1.0f);
}
