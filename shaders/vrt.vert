#version 430

in vec2 vpos;
in vec2 sh_tex_coord;
out vec2 o_tex_coord;

layout (location = 2) uniform mat4 size = mat4(1);
uniform mat4 transform = mat4(1);
uniform vec2 diff = vec2(0,0);


out vec2 pos;

void main() {
    o_tex_coord = sh_tex_coord;
    gl_Position = size*transform*vec4(vpos, 0.0, 1.0);
}
