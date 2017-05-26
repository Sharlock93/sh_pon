#version 430
in vec2 o_tex_coord;

out vec4 color;
uniform vec4 input_color = vec4(1, 1, 1, 1);
uniform sampler2D tex;
uniform bool has_texture = false;

in vec4 fcolor;
void main() {
    vec4 tex_stuff = texture(tex, o_tex_coord);
    if(has_texture) {
        color = vec4(input_color.xyz, tex_stuff.r);
    } else {
        color = vec4(input_color.xyz, 1);
    }
}
