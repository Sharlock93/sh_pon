#version 450

layout (location = 0) in vec2 vpos;
layout (location = 2) uniform mat4 size;


out vec2 pos;

void main() {
    gl_Position = vec4(vpos, 0.0, 1.0);
}
