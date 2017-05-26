#include "../header/game_debug.h"
// #include <stdio.h>

void sh_memcpy(uint8 *dest_mem, uint8 *source, uint32 bytes_to_cpy) {
    while(bytes_to_cpy--) {
        *( dest_mem + bytes_to_cpy) = *(source + bytes_to_cpy);
    }
}

void sh_memset(uint8 *mem, uint8 data, uint32 byte_size) {
    while(byte_size-- > 0){
        *mem++ = data;
    }
}

uint32 sh_streq(const char *str1, const char *str2, uint32 bytes_to_cmp) {
    while(( bytes_to_cmp > 0 ) && ( *str1 != '\0' ) && ( *str2  != '\0' ) && (*str1 == *str2)) {
        ++str1;
        ++str2;
        --bytes_to_cmp;
    }

    return bytes_to_cmp == 0;
}

uint8* sh_cpy_str(uint8 *start, uint32 length) {
    uint8 *new_str = (uint8 *) malloc(length + 1); 
    sh_memcpy(new_str, start, length);
    *( new_str + length) = '\0';
    
    return new_str;
}

int32 sh_parse_int(uint8 *mem, uint8 str_len) {
    int32 number = 0;
    int32 sign = 1;
    if(*mem == '-') {
        sign = -1;
        str_len--;
        mem++;
    }
    
    while(str_len-- > 0) {
        number = number*10 + (*mem - '0');
        mem++;
    }

    return sign*number;

}

vec4 sh_parse_vec4(uint8 *start, uint32 len) {
    
    vec4 vec;
    uint8 index = 0;
    uint8 *val_start = start;
    uint8 *val_end = start;
    while(val_end != (start + len)) {
        while(*val_end != ','  && val_end != (start + len)) val_end++;
        vec[index++] = sh_parse_int(val_start, val_end - val_start);
        if(val_end == (start + len)) break;
        val_end++;
        val_start = val_end; 
    }

    return vec;
}

vec2 sh_parse_vec2(uint8 *start, uint32 len) {
    vec2 vec;
    uint8 index = 0;
    uint8 *val_start = start;
    uint8 *val_end = start;
    while(val_end != (start + len)) {
        while(*val_end != ','  && val_end != (start + len)) val_end++;
        vec[index++] = sh_parse_int(val_start, val_end - val_start);
        if(val_end == (start + len)) break;
        val_end++;
        val_start = val_end; 
    }

    return vec;
}

int32 sh_abs(int32 number) {
    return ( number > 0) ? number : -number;
}

void render_rect(vec2 pos, float width, float height, vec4 color, int vpos_attrib, int color_attrib) {
    vec2 vertex[] = {
        vec2(pos.x - width/2.0, pos.y + height/2.0), 
        vec2(pos.x + width/2.0, pos.y + height/2.0), 
        vec2(pos.x + width/2.0, pos.y - height/2.0), 
        vec2(pos.x - width/2.0, pos.y - height/2.0)
    };

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*4, vertex, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vpos_attrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glUniform4fv(color_attrib, 1, &color[0]);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);
    
}


int sh_button(game_state *gs, unsigned int id, vec2 position, char *text, vec4 color) {
    int32 text_length = sh_strlen((uint8 *)text)-1;
    float width = 0;
    float height = 20;

    float scale = sh_get_scale_for_pixel(&gs->font, 20);

    for(int32 i = 0; i < text_length; ++i) {
        fnt_char *ch = gs->font.characters + text[i];
        width += ch->xadvance*scale;
    }

    input_state *inputs = gs->inputs;
    sh_ui_state *state = &gs->ui_state;
    vec2 mouse = vec2(inputs->mouse.mouse_x, inputs->mouse.mouse_y);
    draw_element rect = sh_draw_rect_fill(position, width + 10, height + 10, color);

    int clicked = 0;

    sh_rect_container rect_cont = {position, width, height};

    if(state->active_object == id) {
        rect.color = vec4(0, 1, 1, 1);
    }

    if(point_in_rect(&mouse, &rect_cont)) {
        state->hot_object = id;
        if(inputs->mouse.left_button) {
            state->active_object = id;
            rect.color = vec4(1, 0, 0, 1);
            clicked = 1;
        } else {
            rect.color = vec4(1, 1, 0, 1);
        }
    }

    push_draw_text(gs, text, height, vec2(position.x - width/2.0, position.y - height/2.0), vec4(0, 0, 0, 1));
    push_draw_element(&gs->renderstack, rect);

    return clicked;
}

int sh_button_circ(game_state *gs, unsigned int id, vec2 position, char *text, float r) {
    input_state *inputs = gs->inputs;
    sh_ui_state *state = &gs->ui_state;

    vec2 mouse = vec2(inputs->mouse.mouse_x, inputs->mouse.mouse_y);
    draw_element circ = sh_draw_circ(position, r, vec4(1, 1, 1, 1));

    int clicked = 0;
    sh_circ_container circ_cont = {position, r};

    if(state->active_object == id) {
        circ.color = vec4(0, 1, 1, 1);
    }

    if(point_in_circ(&mouse, &circ_cont)) {
        state->hot_object = id;
        if(inputs->mouse.left_button) {
            state->active_object = id;
            circ.color = vec4(1, 0, 0, 1);
            clicked = 1;
        } else {
            circ.color = vec4(1, 1, 0, 1);
        }
    }

    push_draw_element(&gs->renderstack, circ);

    return clicked;
}


char* sh_inttstr(int32 val) {
    static char buffer[12] = {};

    char *result = buffer + 11;
    do {
        result--;
        int8 curr_dig = val%10;
        *result = '0' + curr_dig; 
        val /= 10;
    } while(val);

    return result;
}

char* sh_flttstr(float val) {
    //Todo(sharo): implement this
    static char buffer[12] = {};
    // int32 significant = *(int32 *) &val & (( 1 << 23 ) - 1);
    // int32 exponent = ( *(int32 *) &val & ( (1 << 31) - 1 ) ) >> 23;
    // int32 sign = *(int32 *) &val >> 30;
    //
    // exponent -= 127;
    sprintf_s(buffer, 12, "%.3f", val);
    
    return buffer;
}

char* sh_vec2tstr(vec2 *vec) {
    static char buffer[24] = {};
    sprintf_s(buffer, 24, "vec2(%.3f, %.3f)", vec->x, vec->y);
    return buffer;
}

