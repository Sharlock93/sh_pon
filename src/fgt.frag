#version 450

out vec4 color;
layout(location = 1) uniform vec4 input_color = vec4(1, 1, 1, 1);

in vec4 fcolor;

void main() {
    color = fcolor;
}
