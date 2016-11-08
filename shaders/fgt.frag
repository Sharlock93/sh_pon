#version 430

out vec4 color;
uniform vec4 input_color = vec4(1, 1, 1, 1);

in vec4 fcolor;

void main() {
    color = input_color;
}
